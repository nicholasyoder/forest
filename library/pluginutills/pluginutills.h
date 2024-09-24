#ifndef PLUGINUTILLS_H
#define PLUGINUTILLS_H


#include <QStringList>

enum PluginType {
    APP_PLUGIN,
    SETTINGS_PLUGIN
};

class pluginutills
{
public:
    pluginutills();

    static QStringList get_plugin_paths(PluginType type);

};

#endif // PLUGINUTILLS_H
