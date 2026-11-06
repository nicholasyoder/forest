// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOGINSETTINGS_H
#define LOGINSETTINGS_H

#include <QObject>

#include "../../library/pluginutills/settings_plugin_interface.h"

class QComboBox;
class QLabel;
class QProcess;
class QPushButton;

// Appearance → Lock Screen: wallpaper for the greeter and lockscreen, installed by forest-login-helper.
class LoginSettings : public QObject
{
    Q_OBJECT
public:
    LoginSettings();

    settings_page* get_settings_item(){ return settings_item; }

private slots:
    void load();
    void choose_photo();
    void update_ui();
    void apply();
    void apply_finished(int exit_code);

private:
    QString preview_source() const;

    settings_page *settings_item = nullptr;
    QLabel *preview = nullptr;
    QLabel *status = nullptr;
    QComboBox *mode_box = nullptr;
    QPushButton *apply_button = nullptr;
    QProcess *process = nullptr;

    bool installed_custom = false; // a custom image is installed
    int installed_mode = 0;
    QString pending_image;          // newly chosen file, not yet installed
};

#endif // LOGINSETTINGS_H
