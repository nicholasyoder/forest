// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingsmanager.h"
#include "flogger.h"

#include <QApplication>

#include "../library/fstyleloader/fstyleloader.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FLogger::install("settings");

    a.setStyleSheet(fstyleloader::loadstyle("settings"));

    SettingsManager w;
    if (a.arguments().length() > 1)
        w.set_initial_page(a.arguments()[1]);
    w.show();

    return a.exec();
}
