/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2024 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef SERVICESSETTINGS_H
#define SERVICESSETTINGS_H

#include "../../library/pluginutills/settings_plugin_interface.h"
#include "hotkeysettings.h"

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
};

#endif // SERVICESSETTINGS_H
