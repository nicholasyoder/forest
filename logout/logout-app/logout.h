// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef POWERMAN_H
#define POWERMAN_H

#include <QWidget>
#include <QTimer>
#include <QtDBus>
#include <QApplication>
#include <QKeyEvent>

#include "iconbutton.h"
#include "imagewidget.h"

enum class ActionType {SHUTDOWN, REBOOT, LOGOUT, SUSPEND, HIBERNATE};

class logoutmanager : public QWidget{
    Q_OBJECT
public:
    logoutmanager();
    ~logoutmanager();

public slots:
    void startbackfade();
    void activate(){this->activateWindow();}

private slots:
    void keyPressEvent(QKeyEvent *event);
    void set_initial_focus();
    void start_action(ActionType action);
    void do_action(ActionType action);
    void cancel();

private:
    void setup();
    QList<imagewidget*> background_faders;
    iconbutton *focusbt = nullptr;
    QSettings *settings = nullptr;
};

#endif // POWERMAN_H
