// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lockersettingsplugin.h"

#include "lockerconfig.h"
#include "miscutills.h"

namespace {

QSpinBox *minutesSpin(int max)
{
    QSpinBox *spin = new QSpinBox;
    spin->setRange(0, max);
    spin->setSuffix(" min");
    spin->setSpecialValueText("Never");
    return spin;
}

} // namespace

QList<settings_item*> LockerSettingsPlugin::get_settings_items()
{
    settings_category *category = new settings_category("Lock Screen", "", "system-lock-screen");
    connect(category, &settings_category::opened, this, &LockerSettingsPlugin::load_settings);

    settings_widget_group *idle_group = new settings_widget_group;
    category->add_child(idle_group);
    display_off_spin = minutesSpin(240);
    idle_group->add_child(new settings_widget("Turn displays off after", "idle timeout blank screen", display_off_spin));
    dim_check = new QCheckBox;
    idle_group->add_child(new settings_widget("Dim the screen before turning displays off", "", dim_check));
    lock_on_display_off_check = new QCheckBox;
    idle_group->add_child(new settings_widget("Lock when displays turn off", "", lock_on_display_off_check));

    settings_widget_group *lock_group = new settings_widget_group;
    category->add_child(lock_group);
    lock_on_suspend_check = new QCheckBox;
    lock_group->add_child(new settings_widget("Lock on suspend", "sleep", lock_on_suspend_check));
    locked_display_off_spin = minutesSpin(60);
    lock_group->add_child(new settings_widget("Turn displays off while locked after", "", locked_display_off_spin));

    RunOnce *runner = new RunOnce(1000);
    runner->setParent(this);
    connect(runner, &RunOnce::activated, this, &LockerSettingsPlugin::save_settings);
    auto changed = [this, runner] { if (!loading) runner->try_activate(); };
    for (QSpinBox *spin : {display_off_spin, locked_display_off_spin})
        connect(spin, &QSpinBox::valueChanged, this, changed);
    for (QCheckBox *check : {dim_check, lock_on_display_off_check, lock_on_suspend_check})
        connect(check, &QCheckBox::toggled, this, changed);

    return {category};
}

void LockerSettingsPlugin::load_settings()
{
    loading = true;
    const LockerConfig config = LockerConfig::load();
    display_off_spin->setValue(config.displayOffMinutes);
    dim_check->setChecked(config.dimBeforeDisplayOff);
    lock_on_display_off_check->setChecked(config.lockOnDisplayOff);
    lock_on_suspend_check->setChecked(config.lockOnSuspend);
    locked_display_off_spin->setValue(config.lockedDisplayOffMinutes);
    loading = false;
}

void LockerSettingsPlugin::save_settings()
{
    LockerConfig config;
    config.displayOffMinutes = display_off_spin->value();
    config.dimBeforeDisplayOff = dim_check->isChecked();
    config.lockOnDisplayOff = lock_on_display_off_check->isChecked();
    config.lockOnSuspend = lock_on_suspend_check->isChecked();
    config.lockedDisplayOffMinutes = locked_display_off_spin->value();
    config.save();
}
