#include "nmcontrol.h"

nmcontrol::nmcontrol(){

}

nmcontrol::~nmcontrol(){

}

void nmcontrol::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist)
{
    p_button = new panelbutton;
    p_button->setupIconButton("network-wired-offline");
    layout->addWidget(p_button);

    p_menu = new popupmenu(p_button, CenteredOnWidget);
    foreach (pmenuitem *item, itemlist)
        p_menu->additem(item);

    connect(p_button, &panelbutton::rightclicked, p_menu, &popupmenu::show);

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
