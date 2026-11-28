// SPDX-License-Identifier: LGPL-3.0-or-later

#include "desktopsettings.h"

#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImageReader>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTextStream>

#include "settingsbinder.h"
#include "settingsrow.h"
#include "../desktopconfig.h"

namespace {

struct DefaultShortcut {
    const char *filename;
    const char *name;
    const char *icon;
    const char *url; // empty -> home path
    const char *label;
    const char *description;
};

QString desktop_dir(){
    return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
}

bool set_shortcut(const DefaultShortcut &shortcut, bool enabled){
    const QString dir = desktop_dir();
    const QString path = dir + "/" + shortcut.filename;
    if (!enabled)
        return !QFileInfo::exists(path) || QFile::remove(path);

    if (!QDir().mkpath(dir))
        return false;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return false;
    const QString url = shortcut.url[0] ? QString::fromLatin1(shortcut.url) : QDir::homePath();
    QTextStream out(&f);
    out << "[Desktop Entry]\n"
        << "Type=Link\n"
        << "Name=" << shortcut.name << "\n"
        << "Icon=" << shortcut.icon << "\n"
        << "URL=" << url << "\n";
    f.close();
    if (f.error() != QFile::NoError)
        return false;
    return f.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner
                            | QFileDevice::ReadGroup | QFileDevice::ReadOther);
}

}

DesktopSettings::DesktopSettings(){
}

DesktopSettings::~DesktopSettings(){
}

QList<settings_page*> DesktopSettings::pages(){
    return {make_wallpaper_page(), make_icons_page()};
}

settings_page *DesktopSettings::make_wallpaper_page(){
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

    return wallpaper_page;
}

settings_page *DesktopSettings::make_icons_page(){
    settings_page *page = new settings_page("desktop/icons", "Desktop Icons", "user-desktop");
    page->set_keywords({"desktop", "icons", "grid", "spacing", "size", "sort", "arrange",
                        "hidden", "single", "click", "home", "trash", "computer"});

    SettingsBinder *binder = new SettingsBinder("Forest", "Forest", desktopconfig::group, this);
    binder->set_callback([]{ miscutills::call_dbus_async("forest/desktop/reloadicons"); });
    connect(page, &settings_category::opened, binder, &SettingsBinder::load);

    QCheckBox *show_icons = new QCheckBox;
    binder->bind(show_icons, desktopconfig::showicons, desktopconfig::showicons_default);
    page->add_child(new settings_widget("Show desktop icons", "", show_icons));

    settings_widget_group *layout_group = new settings_widget_group("Layout");
    page->add_child(layout_group);

    QSpinBox *icon_size = new QSpinBox;
    icon_size->setRange(desktopconfig::iconsize_min, desktopconfig::iconsize_max);
    icon_size->setSingleStep(4);
    icon_size->setSuffix(" px");
    binder->bind(icon_size, desktopconfig::iconsize, desktopconfig::iconsize_default);
    layout_group->add_child(new settings_widget("Icon size", "", icon_size));

    QSpinBox *grid_spacing = new QSpinBox;
    grid_spacing->setRange(desktopconfig::gridspacing_min, desktopconfig::gridspacing_max);
    grid_spacing->setSingleStep(2);
    grid_spacing->setSuffix(" px");
    binder->bind(grid_spacing, desktopconfig::gridspacing, desktopconfig::gridspacing_default);
    layout_group->add_child(new settings_widget("Grid spacing", "Space between icons on the desktop grid", grid_spacing));

    settings_widget_group *arrange_group = new settings_widget_group("Arrangement");
    page->add_child(arrange_group);

    QCheckBox *auto_arrange = new QCheckBox;
    binder->bind(auto_arrange, desktopconfig::autoarrange, desktopconfig::autoarrange_default);
    arrange_group->add_child(new settings_widget("Auto-arrange icons",
        "Keep icons sorted automatically instead of dragging them into place", auto_arrange));

    QComboBox *sort_by = new QComboBox;
    sort_by->addItem("Name", "name");
    sort_by->addItem("Type", "type");
    sort_by->addItem("Size", "size");
    sort_by->addItem("Date modified", "modified");
    binder->bind(sort_by, desktopconfig::sortby, desktopconfig::sortby_default, SettingsBinder::item_data(sort_by));
    arrange_group->add_child(new settings_widget("Sort by", "", sort_by));

    QComboBox *sort_order = new QComboBox;
    sort_order->addItem("Ascending", "ascending");
    sort_order->addItem("Descending", "descending");
    binder->bind(sort_order, desktopconfig::sortorder, desktopconfig::sortorder_default, SettingsBinder::item_data(sort_order));
    arrange_group->add_child(new settings_widget("Sort order", "", sort_order));

    QCheckBox *folders_first = new QCheckBox;
    binder->bind(folders_first, desktopconfig::foldersfirst, desktopconfig::foldersfirst_default);
    arrange_group->add_child(new settings_widget("Folders first", "", folders_first));

    QPushButton *arrange_now = new QPushButton("Arrange");
    connect(arrange_now, &QPushButton::clicked, this, [binder, arrange_now]{
        binder->flush();
        miscutills::call_dbus_async("forest/desktop/arrangeicons");
        settingsrow::flash(settingsrow::row_of(arrange_now), "saved", "true", 1000);
    });
    arrange_group->add_child(new settings_widget("Arrange icons now",
        "Sort icons on the desktop using the order above", arrange_now));

    settings_widget_group *behavior_group = new settings_widget_group("Behavior");
    page->add_child(behavior_group);

    QCheckBox *single_click = new QCheckBox;
    binder->bind(single_click, desktopconfig::singleclick, desktopconfig::singleclick_default);
    behavior_group->add_child(new settings_widget("Single-click to open",
        "Open files and folders with a single click instead of double-clicking", single_click));

    QCheckBox *show_hidden = new QCheckBox;
    binder->bind(show_hidden, desktopconfig::showhidden, desktopconfig::showhidden_default);
    behavior_group->add_child(new settings_widget("Show hidden files",
        "Show files and folders whose names start with a dot", show_hidden));

    settings_widget_group *defaults_group = new settings_widget_group("Default icons");
    page->add_child(defaults_group);

    static const DefaultShortcut shortcuts[] = {
        {"Home.desktop", "Home", "user-home", "", "Home", "Add a shortcut to your home folder on the desktop"},
        {"Trash.desktop", "Trash", "user-trash", "trash:///", "Trash", "Add a shortcut to the trash on the desktop"},
        {"Computer.desktop", "Computer", "computer", "/", "Computer", "Add a shortcut to the filesystem root on the desktop"},
    };

    QList<QWidget*> icon_controls = {
        icon_size, grid_spacing, auto_arrange, sort_by, sort_order, folders_first, single_click, show_hidden,
    };
    for (const DefaultShortcut &shortcut : shortcuts) {
        QCheckBox *check = new QCheckBox;
        defaults_group->add_child(new settings_widget(shortcut.label, shortcut.description, check));
        icon_controls.append(check);

        connect(page, &settings_category::opened, this, [check, shortcut]{
            QSignalBlocker blocker(check);
            check->setChecked(QFileInfo::exists(desktop_dir() + "/" + shortcut.filename));
        });
        connect(check, &QCheckBox::toggled, this, [check, shortcut](bool checked){
            const bool ok = set_shortcut(shortcut, checked);
            if (!ok) {
                QSignalBlocker blocker(check);
                check->setChecked(!checked);
            }
            settingsrow::flash(settingsrow::row_of(check), "saved", ok ? "true" : "error", ok ? 1000 : 3000);
        });
    }

    auto sync_visibility = [=]{
        const bool shown = show_icons->isChecked();
        for (QWidget *w : icon_controls)
            w->setVisible(shown);
        arrange_now->setVisible(shown && !auto_arrange->isChecked());
    };
    connect(show_icons, &QCheckBox::toggled, this, sync_visibility);
    connect(auto_arrange, &QCheckBox::toggled, this, sync_visibility);
    connect(page, &settings_category::opened, this, sync_visibility);
    sync_visibility();

    return page;
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
