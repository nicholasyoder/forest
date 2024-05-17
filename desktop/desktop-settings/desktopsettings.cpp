/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2024 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "desktopsettings.h"
#include <QLabel>
#include <QComboBox>

DesktopSettings::DesktopSettings(){

}

DesktopSettings::~DesktopSettings(){
    swidget->deleteLater();
}
/*
QWidget* DesktopSettings::get_settings_widget(){
    if (!swidget)
        swidget = new settingswidget;
    return swidget;
}*/

QList<settings_item*> DesktopSettings::get_settings_items(){
    QList<settings_item*> items;
    settings_category *desktop_cat = new settings_category("Desktop", "", "preferences-desktop-wallpaper");
    items.append(desktop_cat);

    settings_category *wallpaper_cat = new settings_category("Wallpaper", "Change the desktop wallpaper", "preferences-desktop-wallpaper");
    desktop_cat->add_child(wallpaper_cat);

    QLabel *wallpaper_preview = new QLabel();
    settings_widget *preview_widget = new settings_widget("", "", wallpaper_preview);
    wallpaper_cat->add_child(preview_widget);

    QComboBox *wallpaper_mode = new QComboBox();
    wallpaper_mode->addItem("Stretch");
    settings_widget *mode_widget = new settings_widget("", "", wallpaper_mode);
    wallpaper_cat->add_child(mode_widget);

    settings_category *icons_cat = new settings_category("Icons", "Configure desktop icons", "preferences-desktop-icons");
    desktop_cat->add_child(icons_cat);

    return items;
}
