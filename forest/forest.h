// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FOREST_H
#define FOREST_H

#include <QWidget>
#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QProcess>
#include <QPluginLoader>
#include <QIcon>
#include <QtDBus>
#include <QScreen>

#include "app_plugin_interface.h"

#include <xcb/xcb.h>

class forest : public QObject{
    Q_OBJECT

public:
    forest();
    ~forest();

    void setup();
    void XcbEventFilter(xcb_generic_event_t* event);

public slots:
    void loadstylesheet();

private:
    void loadplugins();

    QList<app_plugin_interface*> xcbpluglist;
};




#endif // FOREST_H
