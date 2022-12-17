#include "settingsmanager.h"

settingsmanager::settingsmanager(QString app, QString node, QVariant defaultdata, QString notify)
{
    settings = new QSettings("Forest", app);
    sNode = node;
    sDefaultData = defaultdata;
    sNotify = notify;
}

void settingsmanager::setdata(QVariant data)
{
    settings->setValue(sNode, data);
    settings->sync();

    if (sNotify != ""){
        QString path = sNotify;

        QString slot = path.split("/").last();
        path.remove(slot);
        if (path.endsWith("/")) path.chop(1);

        if (QDBusConnection::sessionBus().isConnected()){
            QDBusInterface iface("org.forest", "/org/" + path, "", QDBusConnection::sessionBus());
            if (iface.isValid()) iface.call(slot);
            else fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
        }
        else {
            fprintf(stderr, "Cannot connect to the D-Bus session bus.\nTo start it, run:\n\teval `dbus-launch --auto-syntax`\n");
        }
    }
    emit datachanged();
}

QVariant settingsmanager::data()
{
    return settings->value(sNode, sDefaultData);
}
