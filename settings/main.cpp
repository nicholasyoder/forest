// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingsmanager.h"
#include "flogger.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDebug>

#include "../library/fstyleloader/fstyleloader.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FLogger::install("settings");

    QString path = a.arguments().value(1);
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (bus.interface()->registerService("org.forest.Settings", QDBusConnectionInterface::DontQueueService,
                                         QDBusConnectionInterface::DontAllowReplacement) != QDBusConnectionInterface::ServiceRegistered) {
        QDBusMessage call = QDBusMessage::createMethodCall("org.forest.Settings", "/org/forest/Settings",
                                                           "org.forest.Settings", "OpenPage");
        call << path << qEnvironmentVariable("XDG_ACTIVATION_TOKEN");
        QDBusMessage reply = bus.call(call);
        if (reply.type() != QDBusMessage::ErrorMessage)
            return 0;
        qWarning() << "Forwarding to running instance failed:" << reply.errorMessage();
    }

    a.setStyleSheet(fstyleloader::loadstyle("settings"));

    SettingsManager w;
    w.set_initial_path(path);
    if (!bus.registerObject("/org/forest/Settings", &w, QDBusConnection::ExportScriptableSlots))
        qCritical() << "Failed to register /org/forest/Settings on DBus:" << bus.lastError().message();
    w.show();

    return a.exec();
}
