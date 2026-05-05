// SPDX-License-Identifier: LGPL-3.0-or-later

#include "servicessettings.h"

ServicesSettings::ServicesSettings()
{

}

QList<settings_item*> ServicesSettings::get_settings_items(){
    QList<settings_item*> items;

    settings_category *services_cat = new settings_category("Services", "", "preferences-system-session-services");
    items.append(services_cat);

    hotkey_settings = new HotkeySettings;
    services_cat->add_child(hotkey_settings->get_settings_item());

    notfications_settings = new NotificationsSettings;
    services_cat->add_child(notfications_settings->get_settings_item());

    return items;
}

ServicesSettings::~ServicesSettings(){

}
