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
    identifyCards = new IdentifyCards(this);

    QTimer::singleShot(5000, this, [this](){
        if (!manager->isReady())
            qWarning() << "Displays: compositor doesn't support wlr-output-management, display profiles disabled";
    });
}

void Displays::onStateChanged(){
    const QList<QString> keys = outputs::identityKeys(manager->state()).values();
    const QSet<QString> connected(keys.begin(), keys.end());
    // Our own applies also end in `done`; only a changed output set is a hotplug.
    if (started && connected == connectedKeys){
        syncActive();
        return;
    }
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
        syncActive();
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
    applying++;
    manager->apply(layout, [this, id = profile.id](OutputManager::Result result){
        applying--;
        if (result == OutputManager::Succeeded) markActive(id);
        else syncActive();
    });
}

void Displays::markActive(const QString &id){
    DisplayProfile *profile = profiles.find(id);
    if (!profile) return; // deleted while applying

    profile->lastUsed = QDateTime::currentDateTime();
    const bool changed = profiles.active() != id;
    profiles.setActive(id);
    profiles.save();

    QString primary = DisplayProfiles::resolvePrimary(*profile, manager->state());
    if (primary.isEmpty()) primary = outputs::topLeft(DisplayProfiles::resolve(*profile, manager->state()));
    setPrimary(primary);

    if (changed) emit activeProfileChanged(id);
}

void Displays::syncActive(){
    if (pending || applying || !manager->isReady()) return;
    const DisplayProfile *current = profiles.find(profiles.active());
    if (current && isLive(*current)) return;

    for (const DisplayProfile &profile : profiles.matching(manager->state())){
        if (isLive(profile)){
            markActive(profile.id);
            return;
        }
    }
    if (!profiles.active().isEmpty()){
        profiles.setActive(QString());
        profiles.save();
        emit activeProfileChanged(QString());
    }
}

bool Displays::isLive(const DisplayProfile &profile) const{
    const OutputState &state = manager->state();
    if (!DisplayProfiles::matches(profile, state)) return false;
    if (!outputs::layoutMatchesState(DisplayProfiles::resolve(profile, state), state)) return false;
    return profile.primary.isEmpty() || DisplayProfiles::resolvePrimary(profile, state) == currentPrimary();
}

void Displays::setPrimary(const QString &name){
    QSettings settings("Forest", "Forest");
    if (name.isEmpty() || settings.value("display/primary_screen").toString() == name) return;
    settings.setValue("display/primary_screen", name);
    settings.sync(); // before other processes hear about it
    emit primaryChanged(name);
}

QString Displays::currentPrimary(){
    return QSettings("Forest", "Forest").value("display/primary_screen").toString();
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

bool Displays::parseLayout(const QString &json, const char *caller, OutputLayout *layout, QString *primary) const{
    if (!manager->isReady()){
        qWarning() << "Displays:" << caller << "- no output state";
        return false;
    }
    const OutputState &state = manager->state();

    OutputLayout requested;
    if (!outputs::layoutFromJson(json, &requested, primary)){
        qWarning() << "Displays:" << caller << "- unparsable layout" << json;
        return false;
    }

    // Complete it: every connected head, keys from the live state, unmentioned heads disabled.
    const QHash<QString, QString> keys = outputs::identityKeys(state);
    for (const OutputConfig &config : std::as_const(requested)){
        if (!keys.contains(config.connector)){
            qWarning() << "Displays:" << caller << "- unknown connector" << config.connector;
            return false;
        }
    }
    layout->clear();
    for (OutputConfig config : outputs::currentLayout(state)){
        auto it = std::find_if(requested.begin(), requested.end(),
                               [&](const OutputConfig &c){ return c.connector == config.connector; });
        if (it != requested.end()) config = *it;
        else config.enabled = false;
        config.key = keys[config.connector];
        *layout << config;
    }

    if (!outputs::isConnected(*layout)){
        qWarning() << "Displays:" << caller << "- no enabled output, or a gap between outputs";
        return false;
    }

    auto enabled = [&](const QString &connector){
        return std::any_of(layout->begin(), layout->end(), [&](const OutputConfig &c){
            return c.enabled && c.connector == connector; });
    };
    if (!enabled(*primary))
        *primary = enabled(currentPrimary()) ? currentPrimary() : outputs::topLeft(*layout);
    return true;
}

bool Displays::applyLayout(const QString &json){
    OutputLayout layout;
    QString primary;
    if (!parseLayout(json, "applyLayout", &layout, &primary)) return false;
    const OutputState &state = manager->state();

    // A primary-only change needs no modeset.
    if (!pending && outputs::layoutMatchesState(layout, state)){
        qInfo() << "Displays: setting the primary display to" << primary;
        setPrimary(primary);
        syncActive();
        emit layoutKept();
        return true;
    }

    // Against the original layout, so a follow-up edit can't skip confirming the first.
    const OutputLayout before = pending ? revertTarget : outputs::currentLayout(state);
    const QString beforePrimary = pending ? revertPrimary : currentPrimary();
    qInfo() << "Displays: applying edited layout";
    applying++;
    manager->apply(layout, [this, before, beforePrimary, layout, primary](OutputManager::Result result){
        applying--;
        if (result != OutputManager::Succeeded){
            syncActive();
            emit applyFailed();
            return;
        }
        setPrimary(primary);
        if (outputs::needsConfirm(before, layout)){
            startConfirm(before, beforePrimary, layout);
        }
        else {
            if (pending) endConfirm();
            syncActive();
            emit layoutKept();
        }
    });
    return true;
}

void Displays::startConfirm(const OutputLayout &revertTo, const QString &revertPrimary, const OutputLayout &applied){
    revertTarget = revertTo;
    this->revertPrimary = revertPrimary;
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
    qInfo() << "Displays: kept the new layout";
    syncActive();
    emit layoutKept();
}

void Displays::revertLayout(){
    if (!pending) return;
    endConfirm();
    qInfo() << "Displays: reverting to the previous layout";
    applying++;
    manager->apply(revertTarget, [this](OutputManager::Result result){
        applying--;
        // Failure here (e.g. cancelled by a hotplug) is left to auto-pick.
        if (result == OutputManager::Succeeded) setPrimary(revertPrimary);
        else qWarning() << "Displays: revert failed";
        syncActive();
        emit layoutReverted();
    });
}

bool Displays::saveProfile(const QString &id, const QString &json){
    DisplayProfile *profile = profiles.find(id);
    if (!profile){
        qWarning() << "Displays: saveProfile: no profile" << id;
        return false;
    }
    OutputLayout layout;
    QString primary;
    if (!parseLayout(json, "saveProfile", &layout, &primary)) return false;

    profile->outputs = layout;
    profile->primary = primary;
    profiles.save();
    qInfo() << "Displays: saved profile" << profile->name;
    emit profilesChanged();
    syncActive();
    return true;
}

QString Displays::saveProfileAs(const QString &name, const QString &json){
    DisplayProfile profile;
    if (!parseLayout(json, "saveProfileAs", &profile.outputs, &profile.primary)) return QString();
    profile.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    profile.name = name.trimmed().isEmpty() ? DisplayProfiles::defaultName(profile.outputs) : name.trimmed();

    profiles.add(profile);
    profiles.save();
    qInfo() << "Displays: created profile" << profile.name << profile.id;
    emit profilesChanged();
    syncActive();
    return profile.id;
}

bool Displays::renameProfile(const QString &id, const QString &name){
    DisplayProfile *profile = profiles.find(id);
    if (!profile || name.trimmed().isEmpty()){
        qWarning() << "Displays: renameProfile: no profile" << id << "or empty name";
        return false;
    }
    profile->name = name.trimmed();
    profiles.save();
    emit profilesChanged();
    return true;
}

void Displays::deleteProfile(const QString &id){
    if (!profiles.find(id)){
        qWarning() << "Displays: deleteProfile: no profile" << id;
        return;
    }
    const bool wasActive = profiles.active() == id;
    qInfo() << "Displays: deleting profile" << profiles.find(id)->name;
    profiles.remove(id);
    profiles.save();
    emit profilesChanged();
    if (wasActive) emit activeProfileChanged(QString());
    syncActive(); // an identical profile may now be the active one
}

void Displays::identify(){
    if (!manager->isReady()) return;
    QHash<QString, QString> labels;
    for (const OutputHeadInfo &head : manager->state().heads){
        if (!head.enabled) continue;
        labels[head.name] = !head.model.isEmpty() ? head.model : head.make;
    }
    identifyCards->show(labels);
}
