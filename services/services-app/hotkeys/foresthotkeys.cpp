// SPDX-License-Identifier: LGPL-3.0-or-later

#include "foresthotkeys.h"

foresthotkeys::foresthotkeys(){
}

foresthotkeys::~foresthotkeys(){
    qDeleteAll(hotkeylist);
}

void foresthotkeys::setup(){
    if (!QDBusConnection::sessionBus().registerObject("/org/forest/hotkeys", this, QDBusConnection::ExportAllSlots))
        qCritical() << "Failed to register /org/forest/hotkeys on DBus:" << QDBusConnection::sessionBus().lastError().message();

    portal = new GlobalShortcutsPortal(this);
    connect(portal, &GlobalShortcutsPortal::shortcutActivated, this, &foresthotkeys::dispatch);
    reconcile();
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

// Closes the session rather than just muting dispatch: Biome keeps swallowing
// bound keys otherwise, and hotkey capture in settings needs them.
void foresthotkeys::pauseHotkeys(){
    paused = true;
    reconcile();
}

void foresthotkeys::resumeHotkeys(){
    paused = false;
    reconcile();
}

void foresthotkeys::reloadhotkeys(){
    configChanged = true;
    reconcile();
}

void foresthotkeys::reconcile(){
    if (busy) return;

    if (sessionOpen && (paused || configChanged)) {
        busy = true;
        portal->closeSession([this]() {
            sessionOpen = false;
            busy = false;
            reconcile();
        });
        return;
    }
    if (sessionOpen || paused) return;

    busy = true;
    portal->createSession([this](bool ok) {
        if (!ok) {
            qCritical() << "foresthotkeys: failed to create a GlobalShortcuts portal session";
            busy = false;
            return;
        }
        sessionOpen = true;
        configChanged = false;
        loadhotkeys();
        portal->bindShortcuts(hotkeylist, [this](bool bound) {
            if (!bound) qCritical() << "foresthotkeys: BindShortcuts request failed";
            busy = false;
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
}
