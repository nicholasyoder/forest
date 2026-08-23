// SPDX-License-Identifier: LGPL-3.0-or-later

#include "pluginutills.h"

#include <QSettings>

pluginutills::pluginutills()
{

}

QStringList pluginutills::get_plugin_paths(PluginType type){
    QStringList paths;
    QString plugin_base_path = "/usr/lib/forest/";
    QSettings settings = QSettings("Forest-wayland","Forest");
    settings.beginGroup("plugins");
    foreach(QString key, settings.childGroups()){
        if (!settings.value(key+"/enabled", false).toBool())
            continue;

        if (type == APP_PLUGIN && settings.value(key+"/settings-only", false).toBool())
            continue;

        QString plugin_name = settings.value(key+"/name").toString();
        QString type_str = (type == APP_PLUGIN) ? "app" : "settings";
        QString plugin_path = plugin_base_path + "lib" + plugin_name + "-" + type_str + ".so";
        paths.append(plugin_path);
    }
    settings.endGroup();
    return paths;
}
