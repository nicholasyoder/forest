// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SERVICESSETTINGS_H
#define SERVICESSETTINGS_H

#include "../../library/pluginutills/settings_plugin_interface.h"
#include "hotkeys/hotkeysettings.h"
#include "notifications/notificationssettings.h"

class ServicesSettings : public QObject, settings_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.services.plugin")
    Q_INTERFACES(settings_plugin_interface)

public:
    ServicesSettings();
    ~ServicesSettings();

    QList<settings_page*> pages() override;

private:
    HotkeySettings* hotkey_settings = nullptr;
    NotificationsSettings *notfications_settings = nullptr;
};

#endif // SERVICESSETTINGS_H
