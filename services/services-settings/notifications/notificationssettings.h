// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NOTIFICATIONSSETTINGS_H
#define NOTIFICATIONSSETTINGS_H

#include <QObject>
#include <QSpinBox>
#include <QPushButton>
#include "../../library/pluginutills/settings_plugin_interface.h"

class NotificationsSettings : public QObject {
    Q_OBJECT
public:
    NotificationsSettings();

public slots:
    settings_category* get_settings_item(){ return settings_item; }

private slots:
    void load_settings();
    void save_settings();
    void send_test_notification();

private:
    settings_category *settings_item = nullptr;
    QSpinBox *min_timeout_spinbox = nullptr;
    QSpinBox *max_timeout_spinbox = nullptr;
    QSpinBox *default_timeout_spinbox = nullptr;
    QSpinBox *height_spinbox = nullptr;
    QSpinBox *width_spinbox = nullptr;
};

#endif // NOTIFICATIONSSETTINGS_H
