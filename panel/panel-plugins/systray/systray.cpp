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

#include "systray.h"


systray::systray()
{
}

systray::~systray()
{

}

void systray::setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist)
{
    Q_UNUSED(itemlist);

    fdo_selection_manager = new FdoSelectionManager();
}

void systray::XcbEventFilter(xcb_generic_event_t *event)
{

}

QHash<QString, QString> systray::getpluginfo()
{
    QHash<QString, QString> info;
    info["name"] = "System Tray";
    info["needsXcbEvents"] = "true";
    return info;
}
