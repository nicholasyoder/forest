// SPDX-License-Identifier: LGPL-3.0-or-later

#include "desktopsettings.h"

#include <QHBoxLayout>

DesktopSettings::DesktopSettings(){
}

DesktopSettings::~DesktopSettings(){
}

QList<settings_page*> DesktopSettings::pages(){
    settings_page *wallpaper_page = new settings_page("appearance/wallpaper", "Wallpaper", "preferences-desktop-wallpaper", 10);
    wallpaper_page->set_keywords({"background", "desktop", "image", "photo"});
    connect(wallpaper_page, &settings_category::opened, this, &DesktopSettings::load_wallpaper_settings);

    QFrame *preview_base = new QFrame;
    preview_base->setObjectName("WallpaperPreviewBase");
    QHBoxLayout *preview_h_layout = new QHBoxLayout(preview_base);
    preview_h_layout->setContentsMargins(QMargins(0,0,0,0));
    preview_h_layout->setSpacing(0);
    preview_h_layout->addStretch(1);
    wallpaper_preview = new QLabel();
    wallpaper_preview->setObjectName("WallpaperPreviewImage");
    wallpaper_preview->setScaledContents(true);
    wallpaper_preview->setMaximumSize(400, 250);
    preview_h_layout->addWidget(wallpaper_preview);
    preview_h_layout->addStretch(1);
    settings_widget *preview_item = new settings_widget("", "", preview_base, true);
    wallpaper_page->add_child(preview_item);

    // TODO: add option for solid color and gradient
    QPushButton *browse_button = new QPushButton("Browse");
    open_photo_dialog = new QFileDialog();
    open_photo_dialog->setFileMode(QFileDialog::ExistingFile);
    open_photo_dialog->setNameFilter(tr("Images (*.png *.xpm *.jpg)"));
    connect(open_photo_dialog, &QFileDialog::fileSelected, this, &DesktopSettings::set_wallpaper);
    connect(browse_button, &QPushButton::clicked, open_photo_dialog, &QFileDialog::show);
    settings_widget *wallpaper_select_item = new settings_widget("Choose Photo", "", browse_button);
    wallpaper_page->add_child(wallpaper_select_item);

    wallpaper_mode = new QComboBox();
    wallpaper_mode->addItem("Fill");
    wallpaper_mode->addItem("Fit");
    wallpaper_mode->addItem("Stretch");
    wallpaper_mode->addItem("Tile");
    wallpaper_mode->addItem("Center");
    settings_widget *mode_item = new settings_widget("Display Mode", "", wallpaper_mode);
    wallpaper_page->add_child(mode_item);

    return {wallpaper_page};
}

void DesktopSettings::load_wallpaper_settings(){
    QSettings settings("Forest", "Forest");

    QString wallpaper_path = settings.value("desktop/wallpaper").toString();
    wallpaper_preview->setPixmap(QPixmap(wallpaper_path));
    {
        const QSignalBlocker blocker(wallpaper_mode);
        wallpaper_mode->setCurrentIndex(settings.value("desktop/imagemode", 0).toInt());
    }

    wallpaper_path.remove(wallpaper_path.split("/").last());
    open_photo_dialog->setDirectory(wallpaper_path);

    connect(wallpaper_mode, &QComboBox::currentTextChanged, this, &DesktopSettings::set_wallpaper_mode, Qt::UniqueConnection);
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
