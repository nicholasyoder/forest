// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SUPERVISEDPROCESS_H
#define SUPERVISEDPROCESS_H

#include <QElapsedTimer>
#include <QObject>
#include <QProcess>

// Runs a program and restarts it unless it exits 0, backing off after fast crashes.
class SupervisedProcess : public QObject
{
    Q_OBJECT

public:
    SupervisedProcess(const QString &program, int minDelayMs, QObject *parent = nullptr);

    void start();

private:
    QString program;
    int minDelayMs;
    QProcess process;
    QElapsedTimer uptime;
    int fastCrashes = 0;
};

#endif // SUPERVISEDPROCESS_H
