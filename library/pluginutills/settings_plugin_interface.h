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

#ifndef SETTINGS_PLUGIN_INTERFACE_H
#define SETTINGS_PLUGIN_INTERFACE_H

#include <QObject>
#include <QWidget>

class settings_plugin_infterace {

public:

    // Destructor
    virtual ~settings_plugin_infterace() {}

    // Get settings widget
    virtual QWidget* get_settings_widget() = 0;

    // Get plugin name
    virtual QString get_name() = 0;

    // Get plugin icon
    virtual QString get_icon() = 0;

};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(settings_plugin_infterace, "forest.settings.plugin.interface")
QT_END_NAMESPACE

#endif // SETTINGS_PLUGIN_INTERFACE_H
