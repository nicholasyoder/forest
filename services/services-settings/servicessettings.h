// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SERVICESSETTINGS_H
#define SERVICESSETTINGS_H

#include "../../library/pluginutills/settings_plugin_interface.h"
#include "hotkeys/hotkeysettings.h"
#include "notifications/notificationssettings.h"

class ServicesSettings : public QObject, settings_plugin_infterace
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.services.plugin")
    Q_INTERFACES(settings_plugin_infterace)

public:
    ServicesSettings();
    ~ServicesSettings();

    // settings_plugin_infterace functions
    QList<settings_item*> get_settings_items();

    //QString get_name(){ return "Services"; }
    //QString get_icon(){ return "preferences-system-session-services"; }

private:
    HotkeySettings* hotkey_settings = nullptr;
    NotificationsSettings *notfications_settings = nullptr;
};

#endif // SERVICESSETTINGS_H
