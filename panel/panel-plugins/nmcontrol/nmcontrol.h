/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2023 Nicholas Yoder
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

#ifndef NMCONTROL_H
#define NMCONTROL_H

#include <QWidget>
#include <QVBoxLayout>

#include "panelpluginterface.h"
#include "panelbutton.h"
#include "popup.h"
#include "popupmenu.h"

//#include <KF5/NetworkManagerQt
class nmcontrol : public QObject, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.nmcontrol.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    nmcontrol();
    ~nmcontrol();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){p_button->close(); p_button->deleteLater(); this->deleteLater();}
    void XcbEventFilter(xcb_generic_event_t* /*event*/){}
    QHash<QString, QString> getpluginfo();
    //end plugininterface

private slots:
    void showpopup();

    void state_change();

private:
    panelbutton *p_button = nullptr;
    popupmenu *p_menu = nullptr;
    popup *p_box = nullptr;

};
#endif // NMCONTROL_H
