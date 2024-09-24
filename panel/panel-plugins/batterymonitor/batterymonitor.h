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
