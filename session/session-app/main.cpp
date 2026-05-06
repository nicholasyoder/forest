// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionapp.h"

#include <QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    SessionApp w;
    w.startSession();
    return a.exec();
}
