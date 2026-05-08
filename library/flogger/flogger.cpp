// SPDX-License-Identifier: LGPL-3.0-or-later

#include "flogger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QStandardPaths>

static QFile *s_logFile = nullptr;
static QMutex s_mutex;

static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg){
    const char *level;
    switch (type) {
    case QtDebugMsg:    level = "DEBUG";    break;
    case QtInfoMsg:     level = "INFO";     break;
    case QtWarningMsg:  level = "WARNING";  break;
    case QtCriticalMsg: level = "CRITICAL"; break;
    case QtFatalMsg:    level = "FATAL";    break;
    default:            level = "UNKNOWN";  break;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");

    QString category;
    if (context.category && QLatin1String(context.category) != QLatin1String("default"))
        category = QString(" [%1]").arg(context.category);

    QString line = QString("[%1] [%2]%3 %4").arg(timestamp, level, category, msg);
    QByteArray bytes = (line + "\n").toUtf8();

    QMutexLocker locker(&s_mutex);

    if (s_logFile && s_logFile->isOpen()) {
        s_logFile->write(bytes);
        s_logFile->flush();
    }

    fprintf(stderr, "%s\n", qPrintable(line));

    if (type == QtFatalMsg)
        abort();
}

void FLogger::install(const QString &appName){
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/forest/logs";
    QDir().mkpath(logDir);

    QString logPath = logDir + "/" + appName + ".log";

    QFileInfo fi(logPath);
    if (fi.exists() && fi.size() > 1024 * 1024) {
        QFile::remove(logPath + ".1");
        QFile::rename(logPath, logPath + ".1");
    }

    s_logFile = new QFile(logPath);
    if (!s_logFile->open(QIODevice::Append | QIODevice::Text)) {
        fprintf(stderr, "FLogger: could not open log file: %s\n", qPrintable(logPath));
        delete s_logFile;
        s_logFile = nullptr;
    } else {
        QString header = "\n--- " + appName + " started "
                         + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + " ---\n";
        s_logFile->write(header.toUtf8());
        s_logFile->flush();
    }

    qInstallMessageHandler(messageHandler);
}
