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

#ifndef BUTTON_H
#define BUTTON_H

#include <QWidget>
#include <QLabel>
#include <QIcon>
#include <QHBoxLayout>
#include <QPainter>
#include <QFile>
#include <QMouseEvent>
#include <QPushButton>
#include <QDesktopWidget>
#include <QTimer>
#include <QApplication>
#include <QLabel>
#include <QScreen>

#include "panelbutton.h"
#include "popupmenu.h"

#include "xcbutills/xcbutills.h"

//#include <X11/X.h>
//#include <X11/extensions/Xdamage.h>

//#undef Bool // defined as int in X11/Xlib.h

class button : public panelbutton
{
    Q_OBJECT

public:
    button(xcb_window_t window, QIcon icon, QString type, QString text);
    ~button();

public slots:
    void sethighlight(xcb_window_t win);
    void slotclicked(QMouseEvent *event);
    void updatedata();
    void closebt(){this->close(); this->deleteLater();}
    xcb_window_t xcbwindow(){return btwindow;}

signals:
    void clicked(QMouseEvent *event);
    void sigclicked(int bnum);
    void readytoclose();
    void sigpressed(int bnum);

    void mouseEnter(button *bt);
    void mouseLeave(button *bt);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private slots:
    void raisewindow();
    void maximizewindow();
    void minimizewindow();
    void closewindow();
    void demaximizewindow();
    void resizeifneeded();

    void handleEnterEvent();
    void handleLeaveEvent();
    //void showpopup();
    //void closepopup();

private:
    bool mousepressed = false;
    bool mouseOver = false;

    QString bttext;
    QString bttype;
    xcb_window_t btwindow;
    int btfontsize;

    QPoint startdragpos;

    popupmenu *pmenu = nullptr;

    //QTimer *openptimer = nullptr;
    //QTimer *closeptimer = nullptr;
    //popup *pbox = nullptr;
    //QLabel *scrshotlabel = nullptr;
    //QLabel *wintitlelabel = nullptr;
    //QVBoxLayout *popupvlayout = nullptr;
};

#endif // BUTTON_H
