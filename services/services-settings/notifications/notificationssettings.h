// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NOTIFICATIONSSETTINGS_H
#define NOTIFICATIONSSETTINGS_H

#include <QObject>
#include "../../library/pluginutills/settings_plugin_interface.h"

class NotificationsSettings : public QObject {
    Q_OBJECT
public:
    NotificationsSettings();

public slots:
    settings_page* get_settings_item(){ return settings_item; }

private slots:
    void send_test_notification();

private:
    settings_page *settings_item = nullptr;
};

#endif // NOTIFICATIONSSETTINGS_H
