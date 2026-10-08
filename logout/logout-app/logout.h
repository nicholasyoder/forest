// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef POWERMAN_H
#define POWERMAN_H

#include <QWidget>
#include <QTimer>
#include <QtDBus>
#include <QApplication>
#include <QKeyEvent>

#include "iconbutton.h"
#include "layeroverlay.h"

enum class ActionType {LOCK, SHUTDOWN, REBOOT, LOGOUT, SUSPEND, HIBERNATE};

class logoutmanager : public QWidget{
    Q_OBJECT
public:
    logoutmanager();
    ~logoutmanager();

public slots:
    void set_initial_focus();
    void activate(){this->activateWindow();}

private slots:
    void keyPressEvent(QKeyEvent *event);
    void start_action(ActionType action);
    void cancel();

private:
    void setup();
    void close_overlays();
    QList<QPointer<layeroverlay>> overlays;
    iconbutton *focusbt = nullptr;
    QSettings *settings = nullptr;
};

#endif // POWERMAN_H
