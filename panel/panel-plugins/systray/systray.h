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

//#include "panelbutton.h"
#include "panelpluginterface.h"
#include "trayicon.h"
#include "xcbutills/xcbutills.h"

#include <QtX11Extras/QX11Info>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xdamage.h>
#include <xcb/xcb.h>
#include <xcb/damage.h>

#undef Bool // defined as int in X11/Xlib.h

typedef long unsigned int luint;

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
    void closePlug(){this->close(); deleteLater();}
    void XcbEventFilter(xcb_generic_event_t *event);
    QHash<QString, QString> getpluginfo();
    //end plugininterface

private slots:
    void starttray();
    void stoptray();
    void onIconDestroyed(QObject * icon);
    void clientMessageEvent(xcb_generic_event_t *e);
    int clientMessage(WId _wid, Atom _msg, luint data0, luint data1 = 0, luint data2 = 0, luint data3 = 0, luint data4 = 0) const;
    void addIcon(Window id);
    TrayIcon* findIcon(Window trayId);

private:
    VisualID getVisual();
    void setIconSize(QSize icosize);

    QList<TrayIcon*> mIcons;

    QSize iconsize;
    Display* mDisplay;
    Window mTrayId = 0;
    int mDamageEvent = 0;
    int mDamageError = 0;
    Atom _NET_SYSTEM_TRAY_OPCODE;
    QHBoxLayout *traylayout;
};

#endif // SYSTRAY_H
