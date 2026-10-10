// SPDX-License-Identifier: LGPL-3.0-or-later

#include "pluginutills.h"

#include <QSettings>

pluginutills::pluginutills()
{

}

QStringList pluginutills::get_plugin_paths(){
    QStringList paths;
    QString plugin_base_path = "/usr/lib/forest/";
    QSettings settings = QSettings("Forest","Forest");
    settings.beginGroup("plugins");
    foreach(QString key, settings.childGroups()){
        if (!settings.value(key+"/enabled", false).toBool())
            continue;

        QString plugin_name = settings.value(key+"/name").toString();
        QString plugin_path = plugin_base_path + "lib" + plugin_name + "-app.so";
        paths.append(plugin_path);
    }
    settings.endGroup();
    return paths;
}
