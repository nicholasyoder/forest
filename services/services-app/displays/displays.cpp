// SPDX-License-Identifier: LGPL-3.0-or-later

#include "displays.h"

#include "confirmcards.h"

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

    revertTimer.setSingleShot(true);
    connect(&revertTimer, &QTimer::timeout, this, &Displays::revertLayout);
    cards = new ConfirmCards(this);
    connect(cards, &ConfirmCards::keep, this, &Displays::keepLayout);
    connect(cards, &ConfirmCards::revert, this, &Displays::revertLayout);

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

    if (pending){
        qInfo() << "Displays: outputs changed during a pending confirmation, dropping it";
        endConfirm();
        emit layoutReverted();
    }

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
    // A saved profile replaces an unconfirmed edit; nothing to revert to.
    if (pending){
        endConfirm();
        emit layoutReverted();
    }

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
    profile.primary = defaultPrimary(profile.outputs);

    profiles.add(profile);
    qInfo() << "Displays: saved profile" << profile.name << profile.id;
    markActive(profile.id);
    emit profilesChanged();
    return profile.id;
}

bool Displays::applyLayout(const QString &json){
    if (!manager->isReady()){
        qWarning() << "Displays: applyLayout: no output state";
        return false;
    }
    const OutputState &state = manager->state();

    OutputLayout requested;
    if (!outputs::layoutFromJson(json, &requested)){
        qWarning() << "Displays: applyLayout: unparsable layout" << json;
        return false;
    }

    // Complete it: every connected head, keys from the live state, unmentioned heads disabled.
    const QHash<QString, QString> keys = outputs::identityKeys(state);
    for (const OutputConfig &config : std::as_const(requested)){
        if (!keys.contains(config.connector)){
            qWarning() << "Displays: applyLayout: unknown connector" << config.connector;
            return false;
        }
    }
    OutputLayout layout;
    for (OutputConfig config : outputs::currentLayout(state)){
        auto it = std::find_if(requested.begin(), requested.end(),
                               [&](const OutputConfig &c){ return c.connector == config.connector; });
        if (it != requested.end()) config = *it;
        else config.enabled = false;
        config.key = keys[config.connector];
        layout << config;
    }

    if (!outputs::isConnected(layout)){
        qWarning() << "Displays: applyLayout: no enabled output, or a gap between outputs";
        return false;
    }

    // Against the original layout, so a follow-up edit can't skip confirming the first.
    const OutputLayout before = pending ? revertTarget : outputs::currentLayout(state);
    qInfo() << "Displays: applying edited layout";
    manager->apply(layout, [this, before, layout](OutputManager::Result result){
        if (result != OutputManager::Succeeded){
            emit applyFailed();
            return;
        }
        if (outputs::needsConfirm(before, layout)){
            startConfirm(before, layout);
        }
        else {
            if (pending) endConfirm();
            saveActiveLayout(layout);
            emit layoutKept();
        }
    });
    return true;
}

void Displays::startConfirm(const OutputLayout &revertTo, const OutputLayout &applied){
    revertTarget = revertTo;
    pendingLayout = applied;
    pending = true;
    revertTimer.start(kConfirmSeconds * 1000);
    revertDeadline = QDeadlineTimer(kConfirmSeconds * 1000);

    QStringList enabled;
    for (const OutputConfig &config : applied)
        if (config.enabled) enabled << config.connector;
    cards->show(enabled, revertDeadline);
    emit confirmPending();
}

void Displays::endConfirm(){
    pending = false;
    revertTimer.stop();
    cards->hide();
}

void Displays::keepLayout(){
    if (!pending) return;
    endConfirm();
    saveActiveLayout(pendingLayout);
    qInfo() << "Displays: kept the new layout";
    emit layoutKept();
}

void Displays::revertLayout(){
    if (!pending) return;
    endConfirm();
    qInfo() << "Displays: reverting to the previous layout";
    manager->apply(revertTarget, [this](OutputManager::Result result){
        // Failure here (e.g. cancelled by a hotplug) is left to auto-pick.
        if (result != OutputManager::Succeeded) qWarning() << "Displays: revert failed";
        emit layoutReverted();
    });
}

void Displays::saveActiveLayout(const OutputLayout &layout){
    const OutputState &state = manager->state();
    DisplayProfile *profile = profiles.find(profiles.active());
    if (profile && DisplayProfiles::matches(*profile, state)){
        profile->outputs = layout;
        if (std::none_of(layout.begin(), layout.end(), [&](const OutputConfig &c){
                return c.enabled && c.connector == profile->primary; }))
            profile->primary = defaultPrimary(layout);
        qInfo() << "Displays: updated profile" << profile->name;
        markActive(profile->id);
        emit profilesChanged();
        return;
    }

    DisplayProfile created;
    created.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    created.name = DisplayProfiles::defaultName(state);
    created.outputs = layout;
    created.primary = defaultPrimary(layout);
    profiles.add(created);
    qInfo() << "Displays: created profile" << created.name << created.id;
    markActive(created.id);
    emit profilesChanged();
}

QString Displays::defaultPrimary(const OutputLayout &layout){
    // The current primary if it's enabled, else the top-left output.
    const QString current = QSettings("Forest", "Forest").value("display/primary_screen").toString();
    const OutputConfig *topLeft = nullptr;
    for (const OutputConfig &config : layout){
        if (!config.enabled) continue;
        if (config.connector == current) return current;
        if (!topLeft || config.pos.x() < topLeft->pos.x()
            || (config.pos.x() == topLeft->pos.x() && config.pos.y() < topLeft->pos.y()))
            topLeft = &config;
    }
    return topLeft ? topLeft->connector : QString();
}
