// SPDX-License-Identifier: LGPL-3.0-or-later

#include "generalsettings.h"

#include <QSettings>

#include "miscutills.h"

GeneralSettings::GeneralSettings() {
    settings_item = new settings_category("General", "", "preferences-system");
    connect(settings_item, &settings_category::opened, this, &GeneralSettings::load_settings);

    settings_widget_group *group = new settings_widget_group;
    settings_item->add_child(group);

    xdg_checkbox = new QCheckBox;
    settings_widget *xdg_item = new settings_widget("Launch XDG autostart apps", "", xdg_checkbox);
    group->add_child(xdg_item);

    RunOnce *runner = new RunOnce(1000);
    connect(runner, &RunOnce::activated, this, &GeneralSettings::save_settings);
    connect(xdg_checkbox, &QCheckBox::toggled, runner, [runner](bool){ runner->try_activate(); });
}

void GeneralSettings::load_settings() {
    QSettings settings("Forest", "Session");
    xdg_checkbox->setChecked(settings.value("launch_xdg_autostart", true).toBool());
}

void GeneralSettings::save_settings() {
    QSettings settings("Forest", "Session");
    settings.setValue("launch_xdg_autostart", xdg_checkbox->isChecked());
    settings.sync();
}
