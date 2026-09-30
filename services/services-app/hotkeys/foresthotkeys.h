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
    //called by dbus
    void reloadhotkeys();
    void pauseHotkeys();
    void resumeHotkeys();

private slots:
    void dispatch(QString id);

private:
    void loadhotkeys();
    // Drives the portal session towards the wanted state, one async step at a time.
    void reconcile();

    GlobalShortcutsPortal *portal = nullptr;
    QList<globalhotkey*> hotkeylist;

    // Wanted state
    bool paused = false;
    bool configChanged = true;
    // Actual state
    bool sessionOpen = false;
    bool busy = false;
};

#endif // FORESTHOTKEYS_H
