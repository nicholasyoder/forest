// SPDX-License-Identifier: LGPL-3.0-or-later

#include "systray.h"
#include "trayicon.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QDebug>

namespace {

constexpr char kWatcherService[] = "org.kde.StatusNotifierWatcher";
constexpr char kWatcherPath[] = "/StatusNotifierWatcher";
constexpr char kWatcherInterface[] = "org.kde.StatusNotifierWatcher";

} // namespace

systray::systray()
{
}

systray::~systray()
{
}

void systray::setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist)
{
    Q_UNUSED(itemlist);

    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(QMargins(0,0,0,0));
    mainLayout->setSpacing(0);
    layout->addWidget(this);

    // services-app (which owns the watcher) has no guaranteed load-order
    // relative to panel-app, so this can't just try once at startup - watch
    // for the watcher actually appearing on the bus too, in case it hasn't
    // registered yet by the time this plugin loads.
    auto *watcherAppeared = new QDBusServiceWatcher(kWatcherService, QDBusConnection::sessionBus(),
        QDBusServiceWatcher::WatchForRegistration, this);
    connect(watcherAppeared, &QDBusServiceWatcher::serviceRegistered, this, &systray::registerHost);

    registerHost();
}

QHash<QString, QString> systray::getpluginfo()
{
    QHash<QString, QString> info;
    info["name"] = "systray";
    return info;
}

void systray::registerHost()
{
    if (hostRegistered)
        return;

    QDBusInterface watcher(kWatcherService, kWatcherPath, kWatcherInterface, QDBusConnection::sessionBus());
    if (!watcher.isValid()) {
        qWarning() << "systray: StatusNotifierWatcher unavailable:" << watcher.lastError().message();
        return;
    }

    hostRegistered = true;
    watcher.asyncCall("RegisterStatusNotifierHost", QDBusConnection::sessionBus().baseService());

    QDBusConnection::sessionBus().connect(kWatcherService, kWatcherPath, kWatcherInterface,
        "StatusNotifierItemRegistered", this, SLOT(addItem(QString)));
    QDBusConnection::sessionBus().connect(kWatcherService, kWatcherPath, kWatcherInterface,
        "StatusNotifierItemUnregistered", this, SLOT(removeItem(QString)));

    const QStringList existing = watcher.property("RegisteredStatusNotifierItems").toStringList();
    for (const QString &identifier : existing)
        addItem(identifier);
}

void systray::addItem(const QString &identifier)
{
    if (tIcons.contains(identifier))
        return;

    // identifier is "busName+path" - bus names never contain '/', so
    // everything up to the first one is the service and everything from it
    // onward (itself included) is the object path.
    const int slashIndex = identifier.indexOf('/');
    if (slashIndex < 0)
        return;

    const QString service = identifier.left(slashIndex);
    const QString path = identifier.mid(slashIndex);

    trayicon *icon = new trayicon(service, path);
    tIcons.insert(identifier, icon);
    mainLayout->addWidget(icon);
}

void systray::removeItem(const QString &identifier)
{
    if (trayicon *icon = tIcons.take(identifier)) {
        icon->hide();
        icon->deleteLater();
    }
}
