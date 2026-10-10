// SPDX-License-Identifier: LGPL-3.0-or-later

#include "servicessettings.h"

ServicesSettings::ServicesSettings()
{

}

QList<settings_page*> ServicesSettings::pages(){
    hotkey_settings = new HotkeySettings;
    notfications_settings = new NotificationsSettings;
    return {hotkey_settings->get_settings_item(), notfications_settings->get_settings_item()};
}

ServicesSettings::~ServicesSettings(){

}
