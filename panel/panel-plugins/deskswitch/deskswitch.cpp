#include "deskswitch.h"

deskswitch::deskswitch() {}

deskswitch::~deskswitch() {}

void deskswitch::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist)
{
    basehlayout = new QHBoxLayout(this);
    basehlayout->setMargin(0);
    basehlayout->setSpacing(0);

    layout->addWidget(this);

    deskcount = Xcbutills::getNumDesktops();

    setupbts();

    pmenu = new popupmenu(this, CenteredOnWidget);
    foreach (pmenuitem *item, itemlist)
        pmenu->additem(item);

    connect(this, &deskswitch::rightclicked, pmenu, &popupmenu::show);

    _net_client_list = Xcbutills::atom("_NET_CLIENT_LIST");
    _net_wm_desktop = Xcbutills::atom("_NET_WM_DESKTOP");
    _net_current_desktop = Xcbutills::atom("_NET_CURRENT_DESKTOP");
}

void deskswitch::XcbEventFilter(xcb_generic_event_t *event){
    if (event->response_type == 28){ // PropertyNotify
        int newcount = Xcbutills::getNumDesktops();
        if (newcount != deskcount){
            deskcount = newcount;
            setupbts();
        }
        xcb_client_message_event_t *message = reinterpret_cast<xcb_client_message_event_t *>(event);
        if (message->type == _net_client_list || message->type == _net_wm_desktop){
            QList<xcb_window_t> windows = Xcbutills::getClientList();
            QHash<int, QList<xcb_window_t>> deskwindows;
            foreach (xcb_window_t window, windows)
                deskwindows[Xcbutills::getWindowDesktop(window)] << window;

            foreach (deskbutton *bt, dbuttons)
                bt->setNumDeskWindows(deskwindows[bt->desknumber()].length());
        }
        else if (message->type == _net_current_desktop){
            emit activate(Xcbutills::getCurrentDesktop());
        }
    }
}

QHash<QString, QString> deskswitch::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "Desktop Switcher";
    info["needsXcbEvents"] = "true";
    return info;
}

void deskswitch::setupbts(){
    dbuttons.clear();
    QLayoutItem *child;
    while ((child = basehlayout->takeAt(0)) != nullptr){
        delete child->widget();
        delete child;
    }
    for(int c = 1; c <= deskcount; c++){
        deskbutton *bt = new deskbutton(c);
        basehlayout->addWidget(bt);
        connect(bt, SIGNAL(clicked(int)), this, SLOT(switchtodesk(int)));
        connect(this, SIGNAL(activate(int)), bt, SLOT(setactive(int)));
        dbuttons << bt;
    }
    emit activate(Xcbutills::getCurrentDesktop());
}

void deskswitch::switchtodesk(int num){
    Xcbutills::setCurrentDesktop(num);
    emit activate(num);
}
