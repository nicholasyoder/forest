// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionsettings.h"

SessionSettings::SessionSettings() {}

SessionSettings::~SessionSettings() {}

QList<settings_page*> SessionSettings::pages() {
    autostart_settings = new AutostartSettings;
    return {autostart_settings->get_settings_item()};
}
