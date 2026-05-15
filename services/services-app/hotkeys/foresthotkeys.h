// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FORESTHOTKEYS_H
#define FORESTHOTKEYS_H

#include <QWidget>
#include <QDebug>
#include <QAbstractEventDispatcher>
#include <QLayout>

#include "hotkey.h"

class foresthotkeys : public QObject
{
    Q_OBJECT

public:
    foresthotkeys();
    ~foresthotkeys();

    void setup();
    void XcbEventFilter(xcb_generic_event_t *event);

public slots:
    void reloadhotkeys();

    //called by dbus
    void showdesktop();
    void pauseHotkeys();
    void resumeHotkeys();

private slots:
    void loadhotkeys();

private:
    QList<globalhotkey*> hotkeylist;
    bool paused = false;
};

#endif // FORESTHOTKEYS_H
