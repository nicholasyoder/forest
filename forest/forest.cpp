// SPDX-License-Identifier: LGPL-3.0-or-later

#include "forest.h"
#include "layeroverlay.h"
#include "menuanchor.h"
#include "pluginutills.h"
#include "../library/fstyleloader/fstyleloader.h"
#include "settings_upgrade_manager.h"

#include <LayerShellQt/Window>
#include <QApplication>
#include <QTimer>

forest::forest(){}

forest::~forest(){}

void forest::setup(){
    SettingsUpgradeManager upgrade_manager(this);
    upgrade_manager.perform_upgrades();

    const QList<layeroverlay*> startup_overlays = layeroverlay::showOnAllScreens(Qt::black, LayerShellQt::Window::LayerOverlay, "forest-startup");

    qApp->installEventFilter(new menuanchor::MenuFilter(this));
    loadstylesheet();
    loadplugins();

    if (!QDBusConnection::sessionBus().registerService("org.forest"))
        qCritical() << "Failed to register org.forest on DBus:" << QDBusConnection::sessionBus().lastError().message();
    if (!QDBusConnection::sessionBus().registerObject("/org/forest", this, QDBusConnection::ExportAllSlots))
        qCritical() << "Failed to register /org/forest object on DBus:" << QDBusConnection::sessionBus().lastError().message();

    // The compositor fades the "forest-startup" namespace out on close,
    // revealing the already-rendered desktop at once.
    for (layeroverlay *overlay : startup_overlays)
        QTimer::singleShot(1000, overlay, &QWidget::close);
}

void forest::loadstylesheet(){
    qApp->setStyleSheet(fstyleloader::loadstyle("forest"));
}

void forest::loadplugins(){
    QStringList plugin_paths = pluginutills::get_plugin_paths(APP_PLUGIN);
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
