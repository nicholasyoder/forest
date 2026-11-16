// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKTOPSETTINGS_H
#define DESKTOPSETTINGS_H

#include <QImage>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSettings>
#include <QFileDialog>

#include "miscutills.h"
#include "../../library/pluginutills/settings_plugin_interface.h"

class DesktopSettings : public QObject, settings_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.desktop.plugin")
    Q_INTERFACES(settings_plugin_interface)

public:
    DesktopSettings();
    ~DesktopSettings();

    QList<settings_page*> pages() override;

private slots:
    void update_preview();

private:
    QLabel *wallpaper_preview = nullptr;
    QComboBox *wallpaper_mode = nullptr;
    QFileDialog *open_photo_dialog = nullptr;
    QString wallpaper_path;
    QString preview_path;
    QImage preview_source;
};

#endif // DESKTOPSETTINGS_H
