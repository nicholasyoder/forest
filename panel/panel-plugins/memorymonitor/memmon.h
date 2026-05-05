// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MEMMON_H
#define MEMMON_H

#include <QWidget>
#include <QMap>
#include <QSettings>
#include <QTimer>
#include <QPainter>
#include <QFile>
#include <QMenu>
#include <QMouseEvent>
#include <QtDBus>

#include "settingswidget.h"
#include "panelpluginterface.h"
#include "panelbutton.h"
#include "popup.h"
#include "graphwidget.h"

class memmon : public panelbutton, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.memorymonitor.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    memmon();
    ~memmon();

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
    void setramop(qreal opacity);
    void setswapop(qreal opacity);

private slots:
    void loadsettings();
    void updatemem();
    void runcommand();

private:
    QSettings *settings = nullptr;
    graphwidget *gwidget;
    popupmenu *pmenu;
    QTimer *refreshtimer = new QTimer;
    int swapbehavior = 1;
    QString clickedcommand;
    QRect displayrect;
    bool verticalpanel = false;
    int margin = 0;
};
#endif // MEMMON_H
