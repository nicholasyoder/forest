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

#ifndef POWERMAN_H
#define POWERMAN_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QProcess>
#include <QMessageBox>
#include <QTimer>
#include <QtDBus>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>

#include "iconbutton.h"
#include "imagewidget.h"


#include <QtX11Extras/QX11Info>

class logoutmanager : public QWidget
{
    Q_OBJECT

public:
    logoutmanager();
    ~logoutmanager();


public slots:
    //void startifmenuclosed();
    //void trystart();
    void startbackfade();
    void activate(){this->activateWindow();}


private:
    void setup();

    QTimer *starttimer = new QTimer;
    QTimer *fallbacktimer = new QTimer;

    qreal opacity = 0.0;
    QTimer *fadetimer = new QTimer;
    imagewidget *backwidget = new imagewidget;
    iconbutton *focusbt;
    QSettings *settings = new QSettings("Forest", "forest-power-manager");


private slots:
    void keyPressEvent(QKeyEvent *event);
    void cancel();
    void shutdown();
    void reboot();
    void logout();
    void suspend();
    void hibernate();

    void startshutdown();
    void startreboot();
    void startlogout();
    void startsuspend();
    void starthibernate();

    //void backfade();
    void showwindow();
    void fadein();
    void fadeout();
    bool dbusCall(const QString &service,const QString &path, const QString &interface,const QString & method, bool sysd=false);
};
#endif // POWERMAN_H
