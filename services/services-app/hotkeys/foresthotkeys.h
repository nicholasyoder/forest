// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FORESTHOTKEYS_H
#define FORESTHOTKEYS_H

#include <QWidget>
#include <QDebug>
#include <QAbstractEventDispatcher>
#include <QLayout>

#include "hotkey.h"
#include "globalshortcutsportal.h"

class foresthotkeys : public QObject
{
    Q_OBJECT

public:
    foresthotkeys();
    ~foresthotkeys();

    void setup();

public slots:
    void reloadhotkeys();

    //called by dbus
    void showdesktop();
    void pauseHotkeys();
    void resumeHotkeys();

private slots:
    void loadhotkeys();
    void dispatch(QString id);

private:
    GlobalShortcutsPortal *portal = nullptr;
    QList<globalhotkey*> hotkeylist;
    bool paused = false;
};

#endif // FORESTHOTKEYS_H
