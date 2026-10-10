// SPDX-License-Identifier: LGPL-3.0-or-later

#include "launcher.h"
#include <qt6xdg/XdgDesktopFile>

#include "xdgactivation.h"

launcher::launcher(int num, QString desktopfilepath) : panelbutton(Icon) {
    lnum = num;
    dfilepath = desktopfilepath;

    XdgDesktopFile deskfile;
    deskfile.load(dfilepath);
    setIcon(deskfile.icon(QIcon::fromTheme("application-x-executable")));
    connect(this, &launcher::leftclicked, this, &launcher::runcommand);
}

void launcher::runcommand(){
    // reload the deskfile instead of preserving the one loaded in the constructor
    // so if the desktop file is changed mid run, this uses the new file.
    XdgDesktopFile deskfile;
    deskfile.load(dfilepath);
    XdgActivation::instance()->request(this);
    XdgActivation::instance()->launch([deskfile](){ deskfile.startDetached(QStringList()); });
}

void launcher::mouseReleaseEvent(QMouseEvent *event){
    setDown(false);

    if (dragged){
        dragged = false;
        emit movefinished();
    }
    else if (event->button() == Qt::LeftButton)
        emit leftclicked();
    else if (event->button() == Qt::RightButton)
        emit rightclicked(lnum);

    emit mouseReleased(event);
}

void launcher::mouseMoveEvent(QMouseEvent *event){
    if (isDown()){
        qreal x = event->position().x();
        if (x < -5){
            dragged = true;
            emit moved(this, true);
        }
        else if (x > this->width() + 5){
            dragged = true;
            emit moved(this, false);
        }
    }
}
