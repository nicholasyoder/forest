// SPDX-License-Identifier: LGPL-3.0-or-later

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
    void paintEvent(QPaintEvent*);
    //void enterEvent(QEnterEvent *){
        //setMouseOver(true);
        //emit enterevent();
        //t->start(100);
    //}

    //void leaveEvent(QEvent *){
        //tryleave();
    //}


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
    QImage getImageNonComposite();
    QSize calculateClientWindowSize();
};

#endif // TRAYICON_H
