// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOGINSETTINGS_H
#define LOGINSETTINGS_H

#include <QObject>

#include "wallpaperconfig.h"
#include "../../library/pluginutills/settings_plugin_interface.h"

class QLabel;
class QProcess;
class QPushButton;
class WallpaperEditor;

// Appearance → Login & Lock Wallpaper: wallpaper for the greeter and lockscreen, installed by forest-login-helper.
class LoginSettings : public QObject
{
    Q_OBJECT
public:
    LoginSettings();

    settings_page* get_settings_item(){ return settings_item; }

private slots:
    void load();
    void update_apply();
    void apply();
    void apply_finished(int exit_code);

private:
    bool has_changes() const;

    settings_page *settings_item = nullptr;
    WallpaperEditor *editor = nullptr;
    QLabel *status = nullptr;
    QPushButton *apply_button = nullptr;
    QProcess *process = nullptr;

    wallpaper::Config installed_config;
};

#endif // LOGINSETTINGS_H
