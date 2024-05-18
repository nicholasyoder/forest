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

#include "panelsettings.h"
#include <QSettings>

PanelSettings::PanelSettings()
{

}

QList<settings_item*> PanelSettings::get_settings_items(){
    QList<settings_item*> items;

    settings_category *panel_cat = new settings_category("Panel", "", "preferences-desktop");
    items.append(panel_cat);

    settings_category *behavior_cat = new settings_category("Behavior", "", "configure");
    connect(behavior_cat, &settings_category::opened, this, &PanelSettings::load_behavior_settings);
    panel_cat->add_child(behavior_cat);

    position_select = new QComboBox();
    position_select->addItem("Top");
    position_select->addItem("Bottom");
    settings_widget *position_item = new settings_widget("Position", "", position_select);
    behavior_cat->add_child(position_item);

    return items;
}

void PanelSettings::load_behavior_settings(){
    QSettings settings("Forest", "Panel");

    QString position = settings.value("position").toString();
    position_select->setCurrentText(position);

    connect(position_select, &QComboBox::currentTextChanged, this, &PanelSettings::set_behavior_settings);
}

void PanelSettings::set_behavior_settings(){
    QSettings settings("Forest", "Panel");
    settings.setValue("position", position_select->currentText());
    settings.sync();

    miscutills::call_dbus("forest/panel/reloadsettings");
}
