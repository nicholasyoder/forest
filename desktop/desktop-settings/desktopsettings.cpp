// SPDX-License-Identifier: LGPL-3.0-or-later

#include "desktopsettings.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QImageReader>

#include "settingsbinder.h"
#include "../desktopconfig.h"

DesktopSettings::DesktopSettings(){
}

DesktopSettings::~DesktopSettings(){
}

QList<settings_page*> DesktopSettings::pages(){
    settings_page *wallpaper_page = new settings_page("appearance/wallpaper", "Wallpaper", "preferences-desktop-wallpaper", 10);
    wallpaper_page->set_keywords({"background", "desktop", "image", "photo", "picture", "fill", "fit", "stretch", "center"});

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
    SettingsBinder *binder = new SettingsBinder("Forest", "Forest", desktopconfig::group, this);
    binder->set_callback([]{ miscutills::call_dbus_async("forest/desktop/reloadwallpaper"); });

    QStringList patterns;
    foreach (QByteArray format, QImageReader::supportedImageFormats())
        patterns << "*." + QString::fromLatin1(format);
    open_photo_dialog = new QFileDialog();
    open_photo_dialog->setFileMode(QFileDialog::ExistingFile);
    open_photo_dialog->setNameFilter(tr("Images (%1)").arg(patterns.join(' ')));
    QPushButton *browse_button = new QPushButton("Browse");
    connect(browse_button, &QPushButton::clicked, open_photo_dialog, &QFileDialog::show);
    // The dialog is the bound control; the button is the row that flashes "saved".
    binder->bind_custom(open_photo_dialog, &QFileDialog::fileSelected,
        [this](QSettings &settings){
            wallpaper_path = settings.value(desktopconfig::wallpaper).toString();
            update_preview();
            open_photo_dialog->setDirectory(QFileInfo(wallpaper_path).path());
        },
        [this](QSettings &settings){ settings.setValue(desktopconfig::wallpaper, open_photo_dialog->selectedFiles().value(0)); });
    connect(open_photo_dialog, &QFileDialog::fileSelected, this, [this](const QString &file){
        wallpaper_path = file;
        update_preview();
    });
    wallpaper_page->add_child(new settings_widget("Choose Photo", "", browse_button));

    wallpaper_mode = new QComboBox();
    for (WALLPAPER_MODE m : {Fill, Fit, Stretch, Center})
        wallpaper_mode->addItem(miscutills::wallpaper_mode_name(m));
        binder->bind(wallpaper_mode, desktopconfig::imagemode, miscutills::wallpaper_mode_name(Fill), {
        [](const QVariant &stored){ return QVariant(miscutills::wallpaper_mode_name(miscutills::wallpaper_mode_from_setting(stored))); },
        [](const QVariant &text){ return text; }});
    connect(wallpaper_mode, &QComboBox::currentTextChanged, this, &DesktopSettings::update_preview);
    wallpaper_page->add_child(new settings_widget("Display Mode", "", wallpaper_mode));

    connect(wallpaper_page, &settings_category::opened, binder, &SettingsBinder::load);

    return {wallpaper_page};
}

void DesktopSettings::update_preview(){
    // Decode only when the file changes.
    if (wallpaper_path != preview_path){
        preview_path = wallpaper_path;
        preview_source = QImage(wallpaper_path);
    }
    const WALLPAPER_MODE mode = miscutills::wallpaper_mode_from_setting(wallpaper_mode->currentText());
    wallpaper_preview->setPixmap(QPixmap::fromImage(
        miscutills::get_wallpaper_scaled(preview_source, mode, wallpaper_preview->maximumSize())));
}
