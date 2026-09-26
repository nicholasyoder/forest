// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionapp.h"
#include "flogger.h"

#include <QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    // See forest/main.cpp's qunsetenv(QT_QPA_PLATFORM) comment - same leak,
    // and session-app is the first process in the chain to inherit it from
    // startforest-wayland, so it needs to stop it here too.
    qunsetenv("QT_QPA_PLATFORM");
    FLogger::install("session");
    SessionApp w;
    w.startSession();
    return a.exec();
}
