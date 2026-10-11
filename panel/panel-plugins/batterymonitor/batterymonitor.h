// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BATTERYMONITOR_H
#define BATTERYMONITOR_H

#define PATH_TO_PS_DIR "/sys/class/power_supply/"

#include <QWidget>
#include <QTimer>
#include <QDir>
#include <QLabel>

#include "panelpluginterface.h"
#include "battery.h"
#include "panelbutton.h"
#include "popup.h"

class batterymonitor : public panelbutton, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.batterymonitor.plugin" FILE "batterymonitor.json")
    Q_INTERFACES(panelpluginterface)

public:
    batterymonitor();
    ~batterymonitor();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<QAction*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    //end plugininterface

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void updatedata();
    void showpopup();

private:
    void updateicon();

    QList<battery*> batterylist;
    // Last lookup's candidates; hasThemeIcon() only runs again when they change.
    QStringList iconcandidates;
    QTimer *updatetimer = new QTimer(this);
    popup *pbox;
    QLabel *popuplabel = new QLabel;
};
#endif // BATTERYMONITOR_H
