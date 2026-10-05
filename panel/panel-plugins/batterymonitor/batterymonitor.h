// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BATTERYMONITOR_H
#define BATTERYMONITOR_H

#define PATH_TO_PS_DIR "/sys/class/power_supply/"

#include <QWidget>
#include <QTimer>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>

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
    void setupPlug(QBoxLayout *layout, QList<QAction*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    QHash<QString, QString> getpluginfo();
    //end plugininterface

private slots:
    void updatedata();
    void showpopup();

private:
    QHBoxLayout *basehlayout = new QHBoxLayout;
    QList<battery*> batterylist;
    QTimer *updatetimer = new QTimer(this);
    popup *pbox;
    QLabel *popuplabel = new QLabel;
};
#endif // BATTERYMONITOR_H
