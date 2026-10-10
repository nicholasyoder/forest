// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYSTEMSETTINGS_H
#define SYSTEMSETTINGS_H

#include "../../library/pluginutills/settings_plugin_interface.h"

#include "aboutpage.h"
#include "forestthemesettings.h"
#include "cursorthemesettings.h"
#include "displays/displayspage.h"

class SystemSettings : public QObject, settings_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.system.plugin")
    Q_INTERFACES(settings_plugin_interface)
public:
    SystemSettings();
    ~SystemSettings();

    QList<settings_page*> pages() override;

private:
    AboutPage *about_page = nullptr;
    CursorThemeSettings *cursor_theme_settings = nullptr;
    ForestThemeSettings *forest_theme_settings = nullptr;
    DisplaysPage *displays_page = nullptr;
};

#endif // SYSTEMSETTINGS_H
