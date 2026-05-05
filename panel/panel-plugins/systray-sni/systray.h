// SPDX-License-Identifier: LGPL-3.0-or-later

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
    Q_PLUGIN_METADATA(IID "forest.panel.systray-sni.plugin")
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
