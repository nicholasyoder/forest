// SPDX-License-Identifier: LGPL-3.0-or-later

#include "foresthotkeys.h"
#include "../../library/xcbutills/xcbutills.h"

foresthotkeys::foresthotkeys(){
}

foresthotkeys::~foresthotkeys(){
}

void foresthotkeys::setup(){
    if (!QDBusConnection::sessionBus().registerObject("/org/forest/hotkeys", this, QDBusConnection::ExportAllSlots))
        qCritical() << "Failed to register /org/forest/hotkeys on DBus:" << QDBusConnection::sessionBus().lastError().message();
    loadhotkeys();
}

void foresthotkeys::XcbEventFilter(xcb_generic_event_t *event){
    if (paused) return;
    foreach (globalhotkey *item, hotkeylist)
        item->XcbEventFilter(event);
}

void foresthotkeys::pauseHotkeys(){
    paused = true;
    for (globalhotkey *item : hotkeylist)
        item->pause();
}

void foresthotkeys::resumeHotkeys(){
    for (globalhotkey *item : hotkeylist)
        item->resume();
    paused = false;
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

        QString action = settings.value("action").toString();
        if (action.startsWith("DBUS:")){
            action.remove("DBUS:");
            QHash<QString, QString> dbushash;
            foreach (QString s, action.split(",")){
                QStringList keyvalue = s.split("=");
                if (keyvalue.length() == 2) dbushash[keyvalue.first()] = keyvalue.last();
            }

            globalhotkey *item = new globalhotkey(kseq, Type_Dbus);
            item->setDbusInfo(dbushash["service"], dbushash["path"], dbushash["interface"], dbushash["method"], dbushash["bus"]);
            hotkeylist.append(item);
        }
        else{
            globalhotkey *item= new globalhotkey(kseq, Type_Exec);
            item->setExecCommand(action);
            hotkeylist.append(item);
        }
        settings.endGroup();
    }

    qInfo() << "Loaded" << hotkeylist.count() << "hotkeys";
}

void foresthotkeys::showdesktop(){
    Xcbutills::showDesktop();
}

void foresthotkeys::reloadhotkeys(){
    while (hotkeylist.length() > 0){
        globalhotkey *shcut = hotkeylist.takeAt(0);
        delete shcut;
    }
    hotkeylist.clear();

    loadhotkeys();
}
