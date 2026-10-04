// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QApplication>
#include <QCommandLineParser>
#include <QSocketNotifier>
#include <QTimer>

#include <csignal>
#include <sys/signalfd.h>
#include <unistd.h>

#include "flogger.h"
#include "fstyleloader.h"
#include "lockscreen.h"

int main(int argc, char *argv[])
{
    // SIGUSR1 unlocks (as in swaylock). Blocked before any thread exists so all inherit the mask.
    sigset_t unlockMask;
    sigemptyset(&unlockMask);
    sigaddset(&unlockMask, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &unlockMask, nullptr);

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

    int unlockFd = signalfd(-1, &unlockMask, SFD_NONBLOCK | SFD_CLOEXEC);
    QSocketNotifier unlockNotifier(unlockFd, QSocketNotifier::Read);
    QObject::connect(&unlockNotifier, &QSocketNotifier::activated, &lockScreen, [unlockFd, &lockScreen] {
        signalfd_siginfo info;
        while (read(unlockFd, &info, sizeof info) == sizeof info) {}
        lockScreen.unlockAndQuit();
    });

#ifdef FOREST_LOCKSCREEN_DEBUG
    if (parser.isSet(unlockAfter)) {
        int secs = parser.value(unlockAfter).toInt();
        QTimer::singleShot(secs * 1000, &lockScreen, &LockScreen::unlockAndQuit);
    }
#endif

    return app.exec();
}
