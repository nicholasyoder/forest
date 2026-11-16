// SPDX-License-Identifier: LGPL-3.0-or-later

#include "forest.h"
#include "layeroverlay.h"
#include "menuanchor.h"
#include "pluginutills.h"
#include "../library/fstyleloader/fstyleloader.h"
#include "settings_upgrade_manager.h"

#include <LayerShellQt/Window>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QTimer>

forest::forest(){}

forest::~forest(){}

void forest::setup(){
    // Claimed first so a second instance exits before touching settings or the screen.
    if (!QDBusConnection::sessionBus().registerService("org.forest")){
        qWarning() << "org.forest is already owned (" << QDBusConnection::sessionBus().lastError().message() << "), exiting";
        ::exit(0);
    }

    SettingsUpgradeManager upgrade_manager(this);
    upgrade_manager.perform_upgrades();

    qApp->setProperty("forestStartupOverlay", true); // read by the notification service
    const QList<QPointer<layeroverlay>> startup_overlays = layeroverlay::showOnAllScreens(Qt::black, layeroverlay::Layer::Overlay, "forest-startup");

    qApp->installEventFilter(new menuanchor::MenuFilter(this));
    loadstylesheet();
    loadplugins();

    if (!QDBusConnection::sessionBus().registerObject("/org/forest", this, QDBusConnection::ExportAllSlots))
        qCritical() << "Failed to register /org/forest object on DBus:" << QDBusConnection::sessionBus().lastError().message();

    // The compositor fades the "forest-startup" namespace out on close,
    // revealing the already-rendered desktop at once.
    QTimer::singleShot(1000, this, [startup_overlays]{
        for (layeroverlay *overlay : startup_overlays)
            if (overlay) overlay->close();
        qApp->setProperty("forestStartupOverlay", false);
    });
}

void forest::loadstylesheet(){
    fstyleloader::apply("forest");
}

void forest::loadplugins(){
    QStringList plugin_paths = pluginutills::get_plugin_paths();
    foreach (QString plugin_path, plugin_paths) {
        QPluginLoader plugloader(plugin_path);
        if (plugloader.load()) {
            QObject *plugin = plugloader.instance();
            if (!plugin) continue;

            app_plugin_interface *pluginterface = qobject_cast<app_plugin_interface *>(plugin);
            if (!pluginterface) continue;

            pluginterface->setupPlug();
        }
        else {
            qDebug() << plugloader.errorString();
        }
    }
}
