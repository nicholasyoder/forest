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

#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QWidget>
#include <QHBoxLayout>

#include "panelpluginterface.h"

#include <xcb/xcb.h>

class KSelectionOwner;
class trayicon;

class systray : public QWidget, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.systray.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    systray();
    ~systray();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){close(); deleteLater();}
    void XcbEventFilter(xcb_generic_event_t *ev);
    QHash<QString, QString> getpluginfo();
    //end plugininterface

private:
    void init();

    void dock(xcb_window_t winId);
    void undock(xcb_window_t winId);
    bool addDamageWatch(xcb_window_t client);

private slots:
    void onClaimedOwnership();
    void onFailedToClaimOwnership();
    void onLostOwnership();
    void setSystemTrayVisual();

private:
    QHBoxLayout *mainLayout = nullptr;

    uint8_t damageEventBase = 0;
    QHash<xcb_window_t, trayicon*> tIcons;
    QHash<xcb_window_t, u_int32_t> tDamageWatches;
    KSelectionOwner *tSelectionOwner;

    bool valid = false;

};

#endif // SYSTRAY_H
