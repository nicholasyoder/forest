// SPDX-License-Identifier: LGPL-3.0-or-later

#include "displays.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QSettings>
#include <QTimer>
#include <QUuid>

#include <algorithm>

void Displays::setup(){
    if (!QDBusConnection::sessionBus().registerObject("/org/forest/displays", this,
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals))
        qCritical() << "Failed to register /org/forest/displays on DBus:" << QDBusConnection::sessionBus().lastError().message();

    profiles.load();

    manager = new OutputManager(this);
    connect(manager, &OutputManager::stateChanged, this, &Displays::onStateChanged);
    connect(&hotplugDebounce, &RunOnce::activated, this, &Displays::autoPick);

    QTimer::singleShot(5000, this, [this](){
        if (!manager->isReady())
            qWarning() << "Displays: compositor doesn't support wlr-output-management, display profiles disabled";
    });
}

void Displays::onStateChanged(){
    const QList<QString> keys = outputs::identityKeys(manager->state()).values();
    const QSet<QString> connected(keys.begin(), keys.end());
    // Our own applies also end in `done`; only a changed output set is a hotplug.
    if (started && connected == connectedKeys) return;
    connectedKeys = connected;

    if (!started){
        started = true;
        autoPick();
    }
    else {
        hotplugDebounce.try_activate();
    }
}

void Displays::autoPick(){
    const QList<DisplayProfile> matches = profiles.matching(manager->state());
    if (matches.isEmpty()){
        qInfo() << "Displays: no profile matches the connected outputs, leaving the layout alone";
        return;
    }
    apply(matches.first());
}

void Displays::apply(const DisplayProfile &profile){
    const OutputLayout layout = DisplayProfiles::resolve(profile, manager->state());
    if (outputs::layoutMatchesState(layout, manager->state())){
        qInfo() << "Displays: profile" << profile.name << "is already the live layout";
        markActive(profile.id);
        return;
    }

    qInfo() << "Displays: applying profile" << profile.name;
    manager->apply(layout, [this, id = profile.id](OutputManager::Result result){
        if (result == OutputManager::Succeeded) markActive(id);
    });
}

void Displays::markActive(const QString &id){
    DisplayProfile *profile = profiles.find(id);
    if (!profile) return; // deleted while applying

    profile->lastUsed = QDateTime::currentDateTime();
    const bool changed = profiles.active() != id;
    profiles.setActive(id);
    profiles.save();

    const QString primary = DisplayProfiles::resolvePrimary(*profile, manager->state());
    if (!primary.isEmpty())
        QSettings("Forest", "Forest").setValue("display/primary_screen", primary);

    if (changed) emit activeProfileChanged(id);
}

void Displays::applyProfile(const QString &id){
    const DisplayProfile *profile = profiles.find(id);
    if (!profile){
        qWarning() << "Displays: applyProfile: no profile" << id;
        return;
    }
    if (!manager->isReady() || !DisplayProfiles::matches(*profile, manager->state())){
        qWarning() << "Displays: applyProfile: profile" << profile->name << "doesn't match the connected outputs";
        return;
    }
    apply(*profile);
}

void Displays::nextProfile(){
    if (!manager->isReady()) return;
    QList<DisplayProfile> matches = profiles.matching(manager->state());
    if (matches.isEmpty()){
        qInfo() << "Displays: nextProfile: no profile matches the connected outputs";
        return;
    }
    std::sort(matches.begin(), matches.end(), [](const DisplayProfile &a, const DisplayProfile &b){
        return QString::localeAwareCompare(a.name, b.name) < 0;
    });

    int index = 0;
    for (int i = 0; i < matches.size(); i++){
        if (matches[i].id == profiles.active()){
            index = (i + 1) % matches.size();
            break;
        }
    }
    apply(matches[index]);
}

QString Displays::saveCurrentAsProfile(const QString &name){
    if (!manager->isReady()){
        qWarning() << "Displays: saveCurrentAsProfile: no output state";
        return QString();
    }
    const OutputState &state = manager->state();

    DisplayProfile profile;
    profile.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    profile.name = name.isEmpty() ? DisplayProfiles::defaultName(state) : name;
    profile.outputs = outputs::currentLayout(state);

    // Keep the current primary if it's enabled, else the top-left output.
    const QString current = QSettings("Forest", "Forest").value("display/primary_screen").toString();
    const OutputHeadInfo *topLeft = nullptr;
    for (const OutputHeadInfo &head : state.heads){
        if (!head.enabled) continue;
        if (head.name == current){
            profile.primary = current;
            break;
        }
        if (!topLeft || head.pos.x() < topLeft->pos.x()
            || (head.pos.x() == topLeft->pos.x() && head.pos.y() < topLeft->pos.y()))
            topLeft = &head;
    }
    if (profile.primary.isEmpty() && topLeft) profile.primary = topLeft->name;

    profiles.add(profile);
    qInfo() << "Displays: saved profile" << profile.name << profile.id;
    markActive(profile.id);
    emit profilesChanged();
    return profile.id;
}
