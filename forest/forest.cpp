#include "forest.h"
#include "fadewidget.h"
#include "pluginutills.h"
#include "../library/fstyleloader/fstyleloader.h"
#include "settings_upgrade_manager.h"

forest::forest(){}

forest::~forest(){}

void forest::setup(){
    SettingsUpgradeManager upgrade_manager(this);
    upgrade_manager.perform_upgrades();

    QList<fadewidget*> fwidgetlist;
    foreach (QScreen *screen, qApp->screens()){
        fadewidget *fwidget = new fadewidget(screen);
        fwidgetlist << fwidget;
        fwidget->show();
    }

    loadstylesheet();
    loadplugins();

    QDBusConnection::sessionBus().registerService("org.forest");
    QDBusConnection::sessionBus().registerObject("/org/forest", this, QDBusConnection::ExportAllSlots);

    foreach (fadewidget* fwidget, fwidgetlist)
        QTimer::singleShot(1000, fwidget, SLOT(start()));
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
