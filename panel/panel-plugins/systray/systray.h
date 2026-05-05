// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QWidget>
#include <QHBoxLayout>

//#include "panelbutton.h"
#include "panelpluginterface.h"
#include "trayicon.h"

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
