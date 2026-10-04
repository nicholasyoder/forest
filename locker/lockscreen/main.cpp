// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QApplication>
#include <QCommandLineParser>
#include <QTimer>

#include "flogger.h"
#include "fstyleloader.h"
#include "lockscreen.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("forest-lockscreen");
    app.setQuitOnLastWindowClosed(false);
    FLogger::install("lockscreen");

    QCommandLineParser parser;
    parser.addHelpOption();
#ifdef FOREST_LOCKSCREEN_DEBUG
    QCommandLineOption unlockAfter("unlock-after", "Debug: unlock after <sec> seconds.", "sec");
    parser.addOption(unlockAfter);
#endif
    parser.process(app);

    app.setStyleSheet(fstyleloader::loadstyle("greeter"));

    LockScreen lockScreen;
    if (!lockScreen.start())
        return 1;

#ifdef FOREST_LOCKSCREEN_DEBUG
    if (parser.isSet(unlockAfter)) {
        int secs = parser.value(unlockAfter).toInt();
        QTimer::singleShot(secs * 1000, &lockScreen, &LockScreen::unlockAndQuit);
    }
#endif

    return app.exec();
}
