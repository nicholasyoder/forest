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

#ifndef CLOCK_H
#define CLOCK_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QDateTime>
#include <QCalendarWidget>
#include <QTimer>
#include <QDebug>
#include <QtDBus>

#include "panelpluginterface.h"
#include "panelbutton.h"
#include "popup.h"
#include "popupmenu.h"

#include "clocksettingswidget.h"

class clockplug : public panelbutton, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.clock.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    clockplug();
    ~clockplug();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    void XcbEventFilter(xcb_generic_event_t* /*event*/){}
    QHash<QString, QString> getpluginfo();
    //end plugininterface

private slots:
    void loadsettings();
    void updatetime();
    void showsettingswidget();
    void showpopup(){popupbox->showpopup();}

private:
    bool twelvehour = false;
    bool showseconds = false;
    QCalendarWidget *cwidget = new QCalendarWidget;
    popup *popupbox;
    popupmenu *pmenu;
};

#endif // CLOCK_H
