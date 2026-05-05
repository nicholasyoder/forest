// SPDX-License-Identifier: LGPL-3.0-or-later

#include "foresthotkeys.h"
#include "../../library/xcbutills/xcbutills.h"

foresthotkeys::foresthotkeys(){
}

foresthotkeys::~foresthotkeys(){
}

void foresthotkeys::setup(){
    QDBusConnection::sessionBus().registerObject("/org/forest/hotkeys", this, QDBusConnection::ExportAllSlots);
    loadhotkeys();
}

void foresthotkeys::XcbEventFilter(xcb_generic_event_t *event){
    foreach (globalhotkey *item, hotkeylist)
        item->XcbEventFilter(event);
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
