// SPDX-License-Identifier: LGPL-3.0-or-later

#include "forest.h"
#include "layeroverlay.h"
#include "pluginutills.h"
#include "../library/fstyleloader/fstyleloader.h"
#include "settings_upgrade_manager.h"

#include <LayerShellQt/Window>
#include <QApplication>
#include <QScreen>
#include <QTimer>

forest::forest(){}

forest::~forest(){}

void forest::setup(){
    SettingsUpgradeManager upgrade_manager(this);
    upgrade_manager.perform_upgrades();

    QList<layeroverlay*> startup_overlays;
    foreach (QScreen *screen, qApp->screens()){
        layeroverlay *overlay = new layeroverlay(QColor(0, 0, 0, 255), LayerShellQt::Window::LayerOverlay, "forest-startup");
        overlay->windowHandle()->setScreen(screen);
        overlay->setFixedSize(screen->size());
        overlay->show();
        startup_overlays << overlay;
    }

    loadstylesheet();
    loadplugins();

    if (!QDBusConnection::sessionBus().registerService("org.forest"))
        qCritical() << "Failed to register org.forest on DBus:" << QDBusConnection::sessionBus().lastError().message();
    if (!QDBusConnection::sessionBus().registerObject("/org/forest", this, QDBusConnection::ExportAllSlots))
        qCritical() << "Failed to register /org/forest object on DBus:" << QDBusConnection::sessionBus().lastError().message();

    // Biome fades this out compositor-side on close() (namespace
    // "forest-startup", see biome/desktop/layer_shell.cpp / fade_config.h) -
    // revealing the wallpaper/panel/icons, which are already fully rendered
    // underneath by now, all at once.
    foreach (layeroverlay* overlay, startup_overlays)
        QTimer::singleShot(1000, overlay, &QWidget::close);
}

void forest::XcbEventFilter(xcb_generic_event_t *event){
    foreach (app_plugin_interface *interface, xcbpluglist)
        interface->XcbEventFilter(event);
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

            if (pluginterface->needs_xcb_events())
                xcbpluglist.append(pluginterface);

            pluginterface->setupPlug();
        }
        else {
            qDebug() << plugloader.errorString();
        }
    }
}
