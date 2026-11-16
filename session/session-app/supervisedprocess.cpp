// SPDX-License-Identifier: LGPL-3.0-or-later

#include "supervisedprocess.h"

#include <QDebug>
#include <QTimer>

#include <iterator>

SupervisedProcess::SupervisedProcess(const QString &program, int minDelayMs, QObject *parent)
    : QObject(parent), program(program), minDelayMs(minDelayMs)
{
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    connect(&process, &QProcess::finished, this, [this](int code, QProcess::ExitStatus status) {
        if (status == QProcess::NormalExit && code == 0)
            return;
        static constexpr int delaysMs[] = {0, 1000, 5000, 30000};
        fastCrashes = uptime.elapsed() < 30000 ? fastCrashes + 1 : 0;
        int delay = qMax(this->minDelayMs, delaysMs[qMin(fastCrashes, int(std::size(delaysMs)) - 1)]);
        qWarning() << this->program << "exited (status" << status << "code" << code << "), restarting in" << delay << "ms";
        QTimer::singleShot(delay, this, &SupervisedProcess::start);
    });
    connect(&process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart)
            qWarning() << "Failed to start" << this->program;
    });
}

void SupervisedProcess::start()
{
    uptime.start();
    process.start(program, QStringList());
}
