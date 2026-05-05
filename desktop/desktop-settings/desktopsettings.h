// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKTOPSETTINGS_H
#define DESKTOPSETTINGS_H

#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSettings>
#include <QFileDialog>

#include "miscutills.h"
#include "../../library/pluginutills/settings_plugin_interface.h"

class DesktopSettings : public QObject, settings_plugin_infterace
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.desktop.plugin")
    Q_INTERFACES(settings_plugin_infterace)

public:
    DesktopSettings();
    ~DesktopSettings();

    QList<settings_item*> get_settings_items();

private slots:
    void load_wallpaper_settings();
    void set_wallpaper(QString file_path);
    void set_wallpaper_mode(QString mode);

private:
    QLabel *wallpaper_preview = nullptr;
    QComboBox *wallpaper_mode = nullptr;
    QFileDialog *open_photo_dialog = nullptr;
};

#endif // DESKTOPSETTINGS_H
