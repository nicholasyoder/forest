#include "windowbutton.h"

#include "xcbutills/xcbutills.h"

windowbutton::windowbutton(ulong windowid, int desktop, QIcon icon, QString text) : window_id(windowid), window_desktop(desktop){
    setupIconAndTextButton(text, icon, 16);

    pmenu = new popupmenu(this, CenteredOnWidget);

    pmenuitem *item = new pmenuitem("Raise", QIcon::fromTheme("arrow-up"));
    connect(item, &pmenuitem::clicked, this, &windowbutton::raise_w);
    pmenu->additem(item);
    pmenuitem *item2 = new pmenuitem("Maximize", QIcon::fromTheme("arrow-up-double"));
    connect(item2, &pmenuitem::clicked, this, &windowbutton::maximize_w);
    pmenu->additem(item2);
    pmenuitem *item3 = new pmenuitem("Demaximize", QIcon::fromTheme("arrow-down"));
    connect(item3, &pmenuitem::clicked, this, &windowbutton::demaximize_w);
    pmenu->additem(item3);
    pmenuitem *item4 = new pmenuitem("Minimize", QIcon::fromTheme("arrow-down-double"));
    connect(item4, &pmenuitem::clicked, this, &windowbutton::minimize_w);
    pmenu->additem(item4);
    pmenuitem *item5 = new pmenuitem("Close", QIcon::fromTheme("application-exit"));
    connect(item5, &pmenuitem::clicked, this, &windowbutton::close_w);
    pmenu->additem(item5);
}

void windowbutton::mouseReleaseEvent(QMouseEvent *event){
    panelbutton::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton){
        raise_w();
    }
    else if(event->button() == Qt::RightButton){
        QSize sizeHint = pmenu->popupw->sizeHint();
        if (width() > sizeHint.width())
            pmenu->popupw->setFixedSize(width(), sizeHint.height());
        else
            pmenu->popupw->setFixedSize(sizeHint.width(), sizeHint.height());

        pmenu->show();
    }
}

void windowbutton::raise_w(){
    Xcbutills::raiseWindow(window_id);
}

void windowbutton::maximize_w(){
    Xcbutills::maximizeWindow(window_id);
}

void windowbutton::minimize_w(){
    Xcbutills::minimizeWindow(window_id);
}

void windowbutton::close_w(){
    Xcbutills::closeWindow(window_id);
}

void windowbutton::demaximize_w(){
    Xcbutills::demaximizeWindow(window_id);
}
