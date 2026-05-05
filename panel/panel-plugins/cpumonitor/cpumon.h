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

#include "settingswidget.h"
#include "panelpluginterface.h"
#include "panelbutton.h"
#include "popup.h"
#include "graphwidget.h"

class cpumon : public panelbutton, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.cpumonitor.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    cpumon();
    ~cpumon();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    void XcbEventFilter(xcb_generic_event_t* /*event*/){}
    QHash<QString, QString> getpluginfo();
    //end plugininterface

public slots:
    void showsettingswidget();
    void reloadcolors();
    void reloadsettings();
    void setbackop(qreal opacity);
    void setforeop(qreal opacity);

private slots:
    void loadsettings();
    void updatecpu();
    void runcommand();

private:
    QSettings *settings = nullptr;
    graphwidget *gwidget;
    popupmenu *pmenu;
    QTimer *refreshtimer = new QTimer;
    QString clickedcommand;
    QRect displayrect;
    bool verticalpanel = false;
    int margin = 0;
    unsigned long long oldvalue = 0;
    unsigned long long oldtotal = 0;
};
#endif // CPUMON_H
