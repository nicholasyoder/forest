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

#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QFile>
#include <QMouseEvent>
#include <QDir>
#include <QSettings>
#include <qt5xdg/XdgDesktopFile>

#include "popupmenu.h"
#include "panelbutton.h"

class launcher : public panelbutton
{
    Q_OBJECT

public:
    launcher(int num, QString desktopfilepath);//, QList<pmenuitem*> itemlist);

    int lnum = 0;
    QString dfilepath;

signals:
    void moved(launcher *l, bool up);
    void movefinished();
    void rightclicked(int num);

public slots:
    void runcommand();
    //void remove();

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *){setDown(true);}

private:
    bool dragged = false;
    XdgDesktopFile deskfile;

};

#endif // LAUNCHER_H
