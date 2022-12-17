/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "foresthotkeys.h"

#include "../library/xcbutills/xcbutills.h"

foresthotkeys::foresthotkeys()
{
}

foresthotkeys::~foresthotkeys()
{

}

void foresthotkeys::setup()
{
    QDBusConnection::sessionBus().registerObject("/org/forest/hotkeys", this, QDBusConnection::ExportAllSlots);
    loadhotkeys();
}

void foresthotkeys::XcbEventFilter(xcb_generic_event_t *event)
{
    foreach (globalhotkey *item, hotkeylist)
    {
        item->XcbEventFilter(event);
    }
}

void foresthotkeys::loadhotkeys()
{
    QSettings settings("Forest","Forest");
    settings.beginGroup("hotkeys");

    foreach (QString hotkey, settings.childGroups())
    {
        settings.beginGroup(hotkey);

        QString action = settings.value("action").toString();
        if (action.startsWith("DBUS:")){
            action.remove("DBUS:");
            QHash<QString, QString> dbushash;
            foreach (QString s, action.split(",")){
                QStringList keyvalue = s.split("=");
                if (keyvalue.length() == 2) dbushash[keyvalue.first()] = keyvalue.last();
            }

            globalhotkey *item = new globalhotkey(QKeySequence(settings.value("keysequence").toString()), Type_Dbus);
            item->setDbusInfo(dbushash["service"], dbushash["path"], dbushash["interface"], dbushash["method"], dbushash["bus"]);
            hotkeylist.append(item);
        }
        else
        {
            globalhotkey *item= new globalhotkey(QKeySequence(settings.value("keysequence").toString()), Type_Exec);
            item->setExecCommand(action);
            hotkeylist.append(item);
        }
        settings.endGroup();
    }
}

void foresthotkeys::showdesktop()
{
    //XShowDesktop::showDesktopNew();
    Xcbutills::showDesktop();
}

void foresthotkeys::reloadhotkeys()
{
    while (hotkeylist.length() > 0)
    {
        globalhotkey *shcut = hotkeylist.takeAt(0);
        delete shcut;
    }
    hotkeylist.clear();

    loadhotkeys();
}
