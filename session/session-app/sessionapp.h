// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SESSIONAPP_H
#define SESSIONAPP_H

#include <QElapsedTimer>
#include <QObject>
#include <QSettings>

class QProcess;

class SessionApp : public QObject
{
    Q_OBJECT

public:
    SessionApp();
    ~SessionApp();

    void startSession();

private slots:
    void launch_autostart_commands();
    void launch_autostart_xdg();

private:
    void startProcess(QString cmd);
    void startLocker();

    QSettings *settings = nullptr;
    QProcess *locker = nullptr;
    QElapsedTimer lockerUptime;
    int lockerFastCrashes = 0;

};
#endif // SESSIONAPP_H
