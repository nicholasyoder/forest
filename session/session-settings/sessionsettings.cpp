// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionsettings.h"

SessionSettings::SessionSettings() {}

SessionSettings::~SessionSettings() {}

QList<settings_item*> SessionSettings::get_settings_items() {
    QList<settings_item*> items;

    settings_category *session_cat = new settings_category("Session", "", "preferences-system-session");
    items.append(session_cat);

    general_settings = new GeneralSettings;
    session_cat->add_child(general_settings->get_settings_item());

    autostart_settings = new AutostartSettings;
    session_cat->add_child(autostart_settings->get_settings_item());

    return items;
}
