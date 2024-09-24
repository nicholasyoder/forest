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

#ifndef TRAYICON_H
#define TRAYICON_H

#include <QFrame>
#include <QTimer>
#include "panelbutton.h"

#include <X11/X.h>
#include <X11/extensions/Xdamage.h>

#define TRAY_ICON_SIZE_DEFAULT 24

class QWidget;
class LXQtPanel;

class TrayIcon: public panelbutton
{
    Q_OBJECT
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)

public:
    TrayIcon(Window iconId, QSize const & iconSize);
    virtual ~TrayIcon();

    Window iconId() { return mIconId; }
    Window windowId() { return mWindowId; }
    void windowDestroyed(Window w);

    QSize iconSize() const { return mIconSize; }
    void setIconSize(QSize iconSize);
    void updateicon();

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent*event);
    void enterEvent(QEvent *){
        //setMouseOver(true);
        //emit enterevent();
        //t->start(100);
    }

    void leaveEvent(QEvent *){
        //tryleave();
    }


//private slots:
    /*void tryleave(){
        if (!geometry().contains(mapFromGlobal(cursor().pos()))){
            setMouseOver(false);
            emit leaveevent();
            t->stop();
        }
    }*/

private:
    void init();

    //QTimer *t = new QTimer;

    QRect iconGeometry();
    Window mIconId;
    Window mWindowId;
    QSize mIconSize;
    Damage mDamage;
    Display* mDisplay;
    QImage iconimage;
    static bool isXCompositeAvailable();
};

#endif // TRAYICON_H
