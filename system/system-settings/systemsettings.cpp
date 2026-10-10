// SPDX-License-Identifier: LGPL-3.0-or-later

#include "systemsettings.h"

SystemSettings::SystemSettings(){
}

QList<settings_page*> SystemSettings::pages(){
    about_page = new AboutPage;
    displays_page = new DisplaysPage;
    forest_theme_settings = new ForestThemeSettings;
    cursor_theme_settings = new CursorThemeSettings;

    return {about_page->get_settings_item(), displays_page->get_settings_item(),
            forest_theme_settings->get_settings_item(), cursor_theme_settings->get_settings_item()};
}

SystemSettings::~SystemSettings(){
    delete about_page;
    delete forest_theme_settings;
    delete cursor_theme_settings;
    delete displays_page;
}


