// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CPUMON_H
#define CPUMON_H

#include <QWidget>
#include <QMap>
#include <QSettings>
#include <QTimer>
#include <QPainter>
#include <QFile>
#include <QMenu>
#include <QMouseEvent>
#include <QtDBus>
#include <QStylePainter>
#include <QStyleOptionButton>

#include "panelpluginterface.h"
#include "panelbutton.h"
#include "popup.h"
#include "graphwidget.h"

class cpumon : public panelbutton, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.cpumonitor.plugin" FILE "cpumonitor.json")
    Q_INTERFACES(panelpluginterface)

public:
    cpumon();
    ~cpumon();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<QAction*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    void reloadSettings(){ loadsettings(); }
    //end plugininterface

private slots:
    void loadsettings();
    void updatecpu();
    void runcommand();

private:
    QSettings *settings = nullptr;
    graphwidget *gwidget;
    QMenu *pmenu = nullptr;
    QTimer *refreshtimer = new QTimer;
    QString clickedcommand;
    QRect displayrect;
    bool verticalpanel = false;
    int margin = 0;
    unsigned long long oldvalue = 0;
    unsigned long long oldtotal = 0;
};
#endif // CPUMON_H
