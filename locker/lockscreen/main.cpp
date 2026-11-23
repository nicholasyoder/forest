// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QApplication>

#include "flogger.h"
#include "fstyleloader.h"
#include "lockscreen.h"
#include "miscutills.h"

int main(int argc, char *argv[])
{
    // SIGUSR1 unlocks (as in swaylock).
    const sigset_t unlockMask = block_signals({SIGUSR1});

    QApplication app(argc, argv);
    app.setApplicationName("forest-lockscreen");
    app.setQuitOnLastWindowClosed(false);
    FLogger::install("lockscreen");

    fstyleloader::apply("login");

    LockScreen lockScreen;
    if (!lockScreen.start())
        return 1;

    watch_signals(unlockMask, &app, [&lockScreen] { lockScreen.unlockAndQuit(); });

    return app.exec();
}
