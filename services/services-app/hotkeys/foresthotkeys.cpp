// SPDX-License-Identifier: LGPL-3.0-or-later

#include "foresthotkeys.h"

foresthotkeys::foresthotkeys(){
}

foresthotkeys::~foresthotkeys(){
}

void foresthotkeys::setup(){
    if (!QDBusConnection::sessionBus().registerObject("/org/forest/hotkeys", this, QDBusConnection::ExportAllSlots))
        qCritical() << "Failed to register /org/forest/hotkeys on DBus:" << QDBusConnection::sessionBus().lastError().message();

    portal = new GlobalShortcutsPortal(this);
    connect(portal, &GlobalShortcutsPortal::shortcutActivated, this, &foresthotkeys::dispatch);

    // Portal setup is inherently async - nothing can be bound until the
    // session exists, so loadhotkeys() (and everything it triggers) waits
    // for createSession()'s callback rather than running synchronously here.
    portal->createSession([this](bool ok) {
        if (!ok) {
            qCritical() << "foresthotkeys: failed to create a GlobalShortcuts portal session";
            return;
        }
        loadhotkeys();
    });
}

void foresthotkeys::dispatch(QString id){
    if (paused) return;
    for (globalhotkey *item : hotkeylist) {
        if (item->id() == id) {
            item->exec();
            return;
        }
    }
}

void foresthotkeys::pauseHotkeys(){
    // A flag toggle alone isn't enough: it only gates dispatch() below, but
    // the portal's bindings are still registered in Biome's compositor-side
    // key-match table, which means Biome still swallows every bound key at
    // the compositor level regardless of what forest does with the
    // Activated signal afterward. edithotkeywidget's whole reason for
    // calling this is to let a raw keypress (e.g. Meta) reach its own
    // capture widget instead - which needs Biome to actually stop
    // intercepting it, i.e. the session's bindings gone, not just muted
    // client-side.
    paused = true;
    portal->closeSession([]() {});
}

void foresthotkeys::resumeHotkeys(){
    portal->createSession([this](bool ok) {
        if (!ok) {
            qCritical() << "foresthotkeys: failed to re-create the GlobalShortcuts portal session on resume";
            paused = false;
            return;
        }
        portal->bindShortcuts(hotkeylist, [this](bool bound) {
            if (!bound) {
                qCritical() << "foresthotkeys: BindShortcuts request failed on resume";
            }
            paused = false;
        });
    });
}

void foresthotkeys::loadhotkeys(){
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

        QString action = settings.value("action").toString();
        if (action.startsWith("DBUS:")){
            action.remove("DBUS:");
            QHash<QString, QString> dbushash;
            foreach (QString s, action.split(",")){
                QStringList keyvalue = s.split("=");
                if (keyvalue.length() == 2) dbushash[keyvalue.first()] = keyvalue.last();
            }

            globalhotkey *item = new globalhotkey(hotkey, description, kseq, Type_Dbus);
            item->setDbusInfo(dbushash["service"], dbushash["path"], dbushash["interface"], dbushash["method"], dbushash["bus"]);
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

    portal->bindShortcuts(hotkeylist, [](bool ok) {
        if (!ok) {
            qCritical() << "foresthotkeys: BindShortcuts request failed";
        }
    });
}

void foresthotkeys::showdesktop(){
    qWarning() << "foresthotkeys::showdesktop: no Biome equivalent to _NET_SHOWING_DESKTOP yet - stubbed out";
}

void foresthotkeys::reloadhotkeys(){
    while (hotkeylist.length() > 0){
        globalhotkey *shcut = hotkeylist.takeAt(0);
        delete shcut;
    }
    hotkeylist.clear();

    portal->closeSession([this]() {
        portal->createSession([this](bool ok) {
            if (!ok) {
                qCritical() << "foresthotkeys: failed to re-create the GlobalShortcuts portal session on reload";
                return;
            }
            loadhotkeys();
        });
    });
}
