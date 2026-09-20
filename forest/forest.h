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

class forest : public QObject{
    Q_OBJECT

public:
    forest();
    ~forest();

    void setup();

public slots:
    void loadstylesheet();

private:
    void loadplugins();
};




#endif // FOREST_H
