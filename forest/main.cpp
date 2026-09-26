// SPDX-License-Identifier: LGPL-3.0-or-later

#include "forest.h"
#include "flogger.h"
#include <QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    FLogger::install("forest");
    forest w;

    w.setup();

    return a.exec();
}
