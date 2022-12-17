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
    QSettings *settings = new QSettings("Forest", "CPU Monitor");
    graphwidget *gwidget;
    popupmenu *pmenu;
    QTimer *refreshtimer = new QTimer;
    QString clickedcommand;
    QRect displayrect;
    bool verticalpanel = false;
    int margin = 0;
    unsigned long long oldvalue;
    unsigned long long oldtotal;
};
#endif // CPUMON_H
