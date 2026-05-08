// SPDX-License-Identifier: LGPL-3.0-or-later

#include "generalsettings.h"

#include <QSettings>

#include "miscutills.h"

GeneralSettings::GeneralSettings() {
    settings_item = new settings_category("General", "", "preferences-system");
    connect(settings_item, &settings_category::opened, this, &GeneralSettings::load_settings);

    settings_widget_group *group = new settings_widget_group;
    settings_item->add_child(group);

    wm_edit = new QLineEdit;
    wm_edit->setPlaceholderText("e.g. xfwm4");
    settings_widget *wm_item = new settings_widget("Window Manager", "", wm_edit);
    group->add_child(wm_item);

    xdg_checkbox = new QCheckBox;
    settings_widget *xdg_item = new settings_widget("Launch XDG autostart apps", "", xdg_checkbox);
    group->add_child(xdg_item);

    RunOnce *runner = new RunOnce(1000);
    connect(runner, &RunOnce::activated, this, &GeneralSettings::save_settings);
    connect(wm_edit, &QLineEdit::textChanged, runner, &RunOnce::try_activate);
    connect(xdg_checkbox, &QCheckBox::toggled, runner, [runner](bool){ runner->try_activate(); });
}

void GeneralSettings::load_settings() {
    QSettings settings("Forest", "Session");
    wm_edit->setText(settings.value("window_manager", "xfwm4").toString());
    xdg_checkbox->setChecked(settings.value("launch_xdg_autostart", true).toBool());
}

void GeneralSettings::save_settings() {
    QSettings settings("Forest", "Session");
    settings.setValue("window_manager", wm_edit->text());
    settings.setValue("launch_xdg_autostart", xdg_checkbox->isChecked());
    settings.sync();
}
