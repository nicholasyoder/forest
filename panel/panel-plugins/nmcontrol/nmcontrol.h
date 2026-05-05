// SPDX-License-Identifier: LGPL-3.0-or-later

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
