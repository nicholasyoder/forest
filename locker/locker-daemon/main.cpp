// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QApplication>

#include "flogger.h"
#include "locker.h"
#include "miscutills.h"

int main(int argc, char *argv[])
{
    // SIGTERM/SIGINT quit cleanly; exit 0 (forest-session won't restart us) unless locked.
    const sigset_t quitMask = block_signals({SIGTERM, SIGINT});

    // QApplication rather than QGuiApplication: the dim overlay is a QWidget.
    QApplication app(argc, argv);
    app.setApplicationName("forest-locker");
    app.setQuitOnLastWindowClosed(false);
    FLogger::install("locker");

    watch_signals(quitMask, &app, [] { QCoreApplication::quit(); });

    Locker locker;
    locker.start();
    app.exec();
    // Quitting kills the lockscreen; a restarted forest-locker relocks.
    return locker.isLocked() ? 1 : 0;
}
