#include "systemsettings.h"

SystemSettings::SystemSettings(){
}

QList<settings_item*> SystemSettings::get_settings_items(){
    QList<settings_item*> items;

    // About
    about_page = new AboutPage;
    items.append(about_page->get_settings_item());

    // Themes
    settings_category *themes_cat = new settings_category("Themes", "", "preferences-desktop-theme");
    items.append(themes_cat);

    forest_theme_settings = new ForestThemeSettings;
    themes_cat->add_child(forest_theme_settings->get_settings_item());

    settings_category *widget_cat = new settings_category("Widget", "", "preferences-desktop-theme");
    themes_cat->add_child(widget_cat);

    settings_category *icon_cat = new settings_category("Icon", "", "preferences-desktop-icons");
    themes_cat->add_child(icon_cat);

    cursor_theme_settings = new CursorThemeSettings;
    themes_cat->add_child(cursor_theme_settings->get_settings_item());

    // System
    //settings_category *system_cat = new settings_category("System", "", "preferences-system");
    //items.append(system_cat);

    return items;
}

SystemSettings::~SystemSettings(){
    delete about_page;
    delete forest_theme_settings;
    delete cursor_theme_settings;
}


