// SPDX-License-Identifier: LGPL-3.0-or-later

#include "nmcontrol.h"

nmcontrol::nmcontrol(){

}

nmcontrol::~nmcontrol(){
    delete p_menu;
}

void nmcontrol::setupPlug(QBoxLayout *layout, QList<QAction*> itemlist)
{
    p_button = new panelbutton(panelbutton::Icon);
    p_button->setIcon("network-wired-offline");
    layout->addWidget(p_button);

    p_menu = new QMenu;
    p_menu->addActions(itemlist);

    connect(p_button, &panelbutton::rightclicked, this, [this]{ popupMenuOnLauncher(p_menu, p_button, CenteredOnWidget); });

    QVBoxLayout *popuplayout = new QVBoxLayout;
    p_box = new popup(popuplayout, p_button, CenteredOnWidget);

    connect(p_button, &panelbutton::leftclicked, this, &nmcontrol::showpopup);
}

QHash<QString, QString> nmcontrol::getpluginfo()
{
    QHash<QString, QString> info;
    info["name"] = "Network Manager";
    return info;
}


void nmcontrol::showpopup(){


    p_box->show();
}
