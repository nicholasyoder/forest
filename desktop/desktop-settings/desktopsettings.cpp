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

#include <QHBoxLayout>

DesktopSettings::DesktopSettings(){
}

DesktopSettings::~DesktopSettings(){
}

QList<settings_item*> DesktopSettings::get_settings_items(){
    QList<settings_item*> items;
    settings_category *desktop_cat = new settings_category("Desktop", "", "preferences-desktop-wallpaper");
    items.append(desktop_cat);

    settings_category *wallpaper_cat = new settings_category("Wallpaper", "Change the desktop wallpaper", "preferences-desktop-wallpaper");
    connect(wallpaper_cat, &settings_category::opened, this, &DesktopSettings::load_wallpaper_settings);
    desktop_cat->add_child(wallpaper_cat);

    QFrame *preview_base = new QFrame;
    preview_base->setObjectName("WallpaperPreviewBase");
    QHBoxLayout *preview_h_layout = new QHBoxLayout(preview_base);
    preview_h_layout->setMargin(0);
    preview_h_layout->setSpacing(0);
    preview_h_layout->addStretch(1);
    wallpaper_preview = new QLabel();
    wallpaper_preview->setObjectName("WallpaperPreviewImage");
    wallpaper_preview->setScaledContents(true);
    wallpaper_preview->setMaximumSize(400, 250);
    preview_h_layout->addWidget(wallpaper_preview);
    preview_h_layout->addStretch(1);
    settings_widget *preview_item = new settings_widget("", "", preview_base, true);
    wallpaper_cat->add_child(preview_item);

    // TODO: add option for solid color and gradient
    QPushButton *browse_button = new QPushButton("Browse");
    open_photo_dialog = new QFileDialog();
    open_photo_dialog->setFileMode(QFileDialog::ExistingFile);
    open_photo_dialog->setNameFilter(tr("Images (*.png *.xpm *.jpg)"));
    connect(open_photo_dialog, &QFileDialog::fileSelected, this, &DesktopSettings::set_wallpaper);
    connect(browse_button, &QPushButton::clicked, open_photo_dialog, &QFileDialog::show);
    settings_widget *wallpaper_select_item = new settings_widget("Choose Photo", "", browse_button);
    wallpaper_cat->add_child(wallpaper_select_item);

    wallpaper_mode = new QComboBox();
    wallpaper_mode->addItem("Fill");
    wallpaper_mode->addItem("Fit");
    wallpaper_mode->addItem("Stretch");
    wallpaper_mode->addItem("Tile");
    wallpaper_mode->addItem("Center");
    settings_widget *mode_item = new settings_widget("Display Mode", "", wallpaper_mode);
    wallpaper_cat->add_child(mode_item);

    settings_category *icons_cat = new settings_category("Icons", "Configure desktop icons", "preferences-desktop-icons");
    desktop_cat->add_child(icons_cat);

    QPushButton *test_button = new QPushButton("Click Me");
    settings_widget *test_item = new settings_widget("Don't do it.", "", test_button);
    icons_cat->add_child(test_item);

    return items;
}

void DesktopSettings::load_wallpaper_settings(){
    QSettings settings("Forest", "Forest");

    QString wallpaper_path = settings.value("desktop/wallpaper").toString();
    wallpaper_preview->setPixmap(QPixmap(wallpaper_path));
    wallpaper_mode->setCurrentIndex(settings.value("desktop/imagemode", 0).toInt());

    wallpaper_path.remove(wallpaper_path.split("/").last());
    open_photo_dialog->setDirectory(wallpaper_path);

    connect(wallpaper_mode, &QComboBox::currentTextChanged, this, &DesktopSettings::set_wallpaper_mode);
}

void DesktopSettings::set_wallpaper(QString file_path){
    wallpaper_preview->setPixmap(QPixmap(file_path));
    QSettings settings("Forest", "Forest");
    settings.setValue("desktop/wallpaper", file_path);
    settings.sync();
    miscutills::call_dbus("forest/desktop/reloadwallpaper");
}

void DesktopSettings::set_wallpaper_mode(QString mode){
    // Convert to int
    QStringList modes;
    modes.append("Fill");
    modes.append("Fit");
    modes.append("Stretch");
    modes.append("Tile");
    modes.append("Center");
    int m = modes.indexOf(mode);
    // Set setting
    QSettings settings("Forest", "Forest");
    settings.setValue("desktop/imagemode", m);
    settings.sync();
    miscutills::call_dbus("forest/desktop/reloadwallpaper");
}
