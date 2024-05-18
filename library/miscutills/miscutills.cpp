#include "miscutills.h"

#include <QProcess>

miscutills::miscutills()
{
}

void miscutills::call_dbus(QString path){
    QString slot = path.split("/").last();
    path = path.remove("/" + slot);

    if (QDBusConnection::sessionBus().isConnected()){
        QDBusInterface iface("org.forest", "/org/" + path, "", QDBusConnection::sessionBus());
        if (iface.isValid()) iface.call(slot);
        else fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }
    else {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\nTo start it, run:\n\teval `dbus-launch --auto-syntax`\n");
    }
}

QString miscutills::run_shell_command(QString command){
    QProcess process;
    process.start("bash", QStringList() << "-c" << command);
    process.waitForFinished();
    return process.readAllStandardOutput();
}
