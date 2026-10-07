// SPDX-License-Identifier: LGPL-3.0-or-later

#include "foresthotkeys.h"

namespace {
constexpr int kMinRetryMs = 1000;
constexpr int kMaxRetryMs = 30000;
}

foresthotkeys::foresthotkeys(){
}

foresthotkeys::~foresthotkeys(){
    qDeleteAll(hotkeylist);
}

void foresthotkeys::setup(){
    if (!QDBusConnection::sessionBus().registerObject("/org/forest/hotkeys", this, QDBusConnection::ExportAllSlots))
        qCritical() << "Failed to register /org/forest/hotkeys on DBus:" << QDBusConnection::sessionBus().lastError().message();

    retryDelayMs = kMinRetryMs;
    retryTimer.setSingleShot(true);
    connect(&retryTimer, &QTimer::timeout, this, &foresthotkeys::reconcile);

    pauserWatcher = new QDBusServiceWatcher(QString(), QDBusConnection::sessionBus(),
        QDBusServiceWatcher::WatchForUnregistration, this);
    connect(pauserWatcher, &QDBusServiceWatcher::serviceUnregistered, this, [this](const QString &name) {
        qWarning() << "foresthotkeys:" << name << "exited while hotkeys were paused, resuming";
        pausers.remove(name);
        pauserWatcher->removeWatchedService(name);
        reconcile();
    });

    portal = new GlobalShortcutsPortal(this);
    connect(portal, &GlobalShortcutsPortal::shortcutActivated, this, &foresthotkeys::dispatch);
    connect(portal, &GlobalShortcutsPortal::sessionLost, this, [this]() {
        sessionOpen = false;
        retryNow();
    });
    connect(portal, &GlobalShortcutsPortal::portalAvailable, this, &foresthotkeys::retryNow);
    reconcile();
}

void foresthotkeys::dispatch(QString id){
    if (!pausers.isEmpty()) return;
    for (globalhotkey *item : hotkeylist) {
        if (item->id() == id) {
            item->exec();
            return;
        }
    }
}

// Closes the session rather than just muting dispatch: Biome keeps swallowing
// bound keys otherwise, and hotkey capture in settings needs them.
void foresthotkeys::pauseHotkeys(){
    const QString caller = calledFromDBus() ? message().service() : QString();
    pausers.insert(caller);
    if (!caller.isEmpty()) pauserWatcher->addWatchedService(caller);
    reconcile();
}

void foresthotkeys::resumeHotkeys(){
    const QString caller = calledFromDBus() ? message().service() : QString();
    pausers.remove(caller);
    if (!caller.isEmpty()) pauserWatcher->removeWatchedService(caller);
    reconcile();
}

void foresthotkeys::reloadhotkeys(){
    configChanged = true;
    reconcile();
}

void foresthotkeys::scheduleRetry(){
    qWarning() << "foresthotkeys: retrying in" << retryDelayMs << "ms";
    retryTimer.start(retryDelayMs);
    retryDelayMs = qMin(retryDelayMs * 2, kMaxRetryMs);
}

void foresthotkeys::retryNow(){
    retryTimer.stop();
    retryDelayMs = kMinRetryMs;
    reconcile();
}

void foresthotkeys::reconcile(){
    if (busy) return;

    const bool paused = !pausers.isEmpty();
    if (sessionOpen && (paused || configChanged)) {
        busy = true;
        portal->closeSession([this]() {
            sessionOpen = false;
            busy = false;
            reconcile();
        });
        return;
    }
    if (sessionOpen || paused || retryTimer.isActive()) return;

    busy = true;
    portal->createSession([this](bool ok) {
        if (!ok) {
            qCritical() << "foresthotkeys: failed to create a GlobalShortcuts portal session";
            busy = false;
            scheduleRetry();
            return;
        }
        sessionOpen = true;
        configChanged = false;
        loadhotkeys();
        portal->bindShortcuts(hotkeylist, [this](bool bound) {
            busy = false;
            if (!bound) {
                qCritical() << "foresthotkeys: BindShortcuts request failed";
                configChanged = true; // next attempt starts from a fresh session
                scheduleRetry();
                return;
            }
            retryDelayMs = kMinRetryMs;
            reconcile();
        });
    });
}

void foresthotkeys::loadhotkeys(){
    qDeleteAll(hotkeylist);
    hotkeylist.clear();

    QSettings settings("Forest","Forest");
    settings.beginGroup("hotkeys");

    foreach (QString hotkey, settings.childGroups()){
        settings.beginGroup(hotkey);

        QKeySequence kseq;
        QString keys = settings.value("keysequence").toString();
        if(keys == "Meta"){
            kseq = QKeySequence(Qt::Key_Meta);
        }
        else{
            kseq = QKeySequence(keys);
        }

        QString description = settings.value("description").toString();

        const QString action = settings.value("action").toString();
        if (const auto dbusAction = hotkeyconfig::parseDBusAction(action)){
            globalhotkey *item = new globalhotkey(hotkey, description, kseq, Type_Dbus);
            item->setDbusAction(*dbusAction);
            hotkeylist.append(item);
        }
        else{
            globalhotkey *item= new globalhotkey(hotkey, description, kseq, Type_Exec);
            item->setExecCommand(action);
            hotkeylist.append(item);
        }
        settings.endGroup();
    }

    qInfo() << "Loaded" << hotkeylist.count() << "hotkeys";
}
