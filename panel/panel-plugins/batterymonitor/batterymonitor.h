// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BATTERYMONITOR_H
#define BATTERYMONITOR_H

//#define PATH_TO_PS_DIR "/home/nicholas/sys/class/power_supply/"
#define PATH_TO_PS_DIR "/sys/class/power_supply/"

#include <QWidget>
#include <QTimer>
#include <QDir>
#include <QHBoxLayout>

#include "panelpluginterface.h"
#include "battery.h"
#include "panelbutton.h"
#include "popup.h"

class batterymonitor : public QWidget, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.batterymonitor.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    batterymonitor();
    ~batterymonitor();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    void XcbEventFilter(xcb_generic_event_t* /*event*/){}
    QHash<QString, QString> getpluginfo();
    //end plugininterface

private slots:
    void updatedata();
    void showpopup();

private:
    bool nobattery = false;
    QHBoxLayout *basehlayout = new QHBoxLayout;
    int currentcapacity = 0;
    int totalcapacity = 0;
    qreal percentfull = 0;
    QList<battery*> batterylist;
    QTimer *updatetimer = new QTimer;
    popup *pbox;
    QLabel *popuplabel = new QLabel;
};
#endif // BATTERYMONITOR_H
