// SPDX-License-Identifier: LGPL-3.0-or-later

#include "forestxcbeventfilter.h"
#include <QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    forest w;

    forestXcbEventFilter *eventfilter = new forestXcbEventFilter;
    eventfilter->f = &w;
    a.installNativeEventFilter(eventfilter);

    w.setup();

    return a.exec();
}
