#ifndef SYSTEMSETTINGS_H
#define SYSTEMSETTINGS_H

#include "../../library/pluginutills/settings_plugin_interface.h"

#include "aboutpage.h"
#include "forestthemesettings.h"
#include "cursorthemesettings.h"

class SystemSettings : public QObject, settings_plugin_infterace
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.system.plugin")
    Q_INTERFACES(settings_plugin_infterace)
public:
    SystemSettings();
    ~SystemSettings();

    QList<settings_item*> get_settings_items();

private:
    AboutPage *about_page = nullptr;
    CursorThemeSettings *cursor_theme_settings = nullptr;
    ForestThemeSettings *forest_theme_settings = nullptr;
};

#endif // SYSTEMSETTINGS_H
