// SPDX-License-Identifier: LGPL-3.0-or-later

#include "forest.h"
#include "flogger.h"
#include <QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    // Qt registers this app ID with the host portal, which GlobalShortcuts requires.
    QGuiApplication::setDesktopFileName("org.forest.Forest");
    // Long-lived shell: closing a transient window (confirm card, popup) mustn't quit it.
    a.setQuitOnLastWindowClosed(false);
    FLogger::install("forest");
    forest w;

    w.setup();

    return a.exec();
}
