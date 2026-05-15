// SPDX-License-Identifier: LGPL-3.0-or-later

#include "greeterwindow.h"
#include "flogger.h"

#include "../../library/fstyleloader/fstyleloader.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    FLogger::install("greeter");
    a.setStyleSheet(fstyleloader::loadstyle("greeter"));
    GreeterWindow w;
    w.showFullScreen();
    return a.exec();
}
