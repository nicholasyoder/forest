#include "settingsmanager.h"

#include <QDebug>
#include <QPluginLoader>
#include "settings_plugin_interface.h"
#include "pluginutills.h"

SettingsManager::SettingsManager(QWidget *parent)
    : QMainWindow(parent)
{
}

SettingsManager::~SettingsManager()
{
}

void SettingsManager::load_settings_ui(){
    QStringList plugin_paths = pluginutills::get_plugin_paths(SETTINGS_PLUGIN);
    foreach (QString plugin_path, plugin_paths) {

        QPluginLoader plugloader(plugin_path);
        if (plugloader.load()) {
            QObject *plugin = plugloader.instance();
            if (!plugin) continue;

            settings_plugin_infterace *pluginterface = qobject_cast<settings_plugin_infterace *>(plugin);
            if (!pluginterface) continue;

            QString plugin_name = pluginterface->get_name();
            QString plugin_icon = pluginterface->get_icon();
            //QWidget *plugin_settings_widget = pluginterface->get_settings_widget();
            qDebug() << plugin_path;
            qDebug() << plugin_name;
            qDebug() << plugin_icon;
        }
        else {
            qDebug() << plugloader.errorString();
        }
    }
}
