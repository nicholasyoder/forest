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

#ifndef DESKTOPSETTINGS_H
#define DESKTOPSETTINGS_H

#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSettings>
#include <QFileDialog>

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

    void call_dbus(QString path);

private:
    QLabel *wallpaper_preview = nullptr;
    QComboBox *wallpaper_mode = nullptr;
    QFileDialog *open_photo_dialog = nullptr;
};

#endif // DESKTOPSETTINGS_H
