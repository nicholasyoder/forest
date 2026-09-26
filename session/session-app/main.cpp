// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionapp.h"
#include "flogger.h"

#include <QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    FLogger::install("session");
    SessionApp w;
    w.startSession();
    return a.exec();
}
