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

#ifndef PANELSETTINGS_H
#define PANELSETTINGS_H

#include <QComboBox>

#include "miscutills.h"
#include "../../library/pluginutills/settings_plugin_interface.h"

class PanelSettings : public QObject, settings_plugin_infterace
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.panel.plugin")
    Q_INTERFACES(settings_plugin_infterace)

public:
    PanelSettings();

    // settings_plugin_infterace functions
    QList<settings_item*> get_settings_items();

public slots:
    void load_behavior_settings();
    void set_behavior_settings();

private:
    QComboBox *position_select = nullptr;
};

#endif // PANELSETTINGS_H
