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

#include "launcher.h"

launcher::launcher(int num, QString desktopfilepath){
    lnum = num;
    dfilepath = desktopfilepath;

    deskfile.load(dfilepath);
    setupIconButton(deskfile.icon(QIcon::fromTheme("application-x-executable")));
    connect(this, &launcher::leftclicked, this, &launcher::runcommand);
}

void launcher::runcommand(){
    deskfile.startDetached(QStringList());
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
        if (event->x() < -5){
            dragged = true;
            emit moved(this, true);
        }
        else if (event->x() > this->width() + 5){
            dragged = true;
            emit moved(this, false);
        }
    }
}
