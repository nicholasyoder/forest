// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QApplication>
#include <QCommandLineParser>
#include <QSocketNotifier>

#include <csignal>
#include <sys/signalfd.h>
#include <unistd.h>

#include "flogger.h"
#include "locker.h"

int main(int argc, char *argv[])
{
    // SIGTERM/SIGINT quit cleanly; exit 0 (forest-session won't restart us) unless locked.
    // Blocked before any thread exists so all inherit the mask.
    sigset_t quitMask;
    sigemptyset(&quitMask);
    sigaddset(&quitMask, SIGTERM);
    sigaddset(&quitMask, SIGINT);
    pthread_sigmask(SIG_BLOCK, &quitMask, nullptr);

    // QApplication rather than QGuiApplication: the dim overlay will be a QWidget.
    QApplication app(argc, argv);
    app.setApplicationName("forest-locker");
    app.setQuitOnLastWindowClosed(false);
    FLogger::install("locker");

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption noLogind("no-logind", "Don't talk to logind (no lock/sleep signals or hints).");
    parser.addOption(noLogind);
    parser.process(app);

    int quitFd = signalfd(-1, &quitMask, SFD_NONBLOCK | SFD_CLOEXEC);
    QSocketNotifier quitNotifier(quitFd, QSocketNotifier::Read);
    QObject::connect(&quitNotifier, &QSocketNotifier::activated, &app, [quitFd] {
        signalfd_siginfo info;
        while (read(quitFd, &info, sizeof info) == sizeof info) {}
        QCoreApplication::quit();
    });

    Locker locker(!parser.isSet(noLogind));
    locker.start();
    app.exec();
    // Quitting kills the lockscreen; a restarted forest-locker relocks.
    return locker.isLocked() ? 1 : 0;
}
