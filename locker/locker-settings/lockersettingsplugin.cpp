// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lockersettingsplugin.h"

#include <QCheckBox>
#include <QSpinBox>

#include "lockerconfig.h"
#include "settingsbinder.h"

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

QList<settings_page*> LockerSettingsPlugin::pages()
{
    settings_page *page = new settings_page("power/lockscreen", "Lock Screen", "system-lock-screen");
    page->set_keywords({"idle", "timeout", "blank", "screen", "sleep", "suspend", "dim"});

    // forest-locker reloads on its own when Locker.conf changes.
    SettingsBinder *binder = new SettingsBinder("Forest", "Locker", QString(), this);
    connect(page, &settings_page::opened, binder, &SettingsBinder::load);
    const LockerConfig defaults;

    settings_widget_group *idle_group = new settings_widget_group("When idle");
    page->add_child(idle_group);
    QSpinBox *display_off_spin = minutesSpin(240);
    binder->bind(display_off_spin, LockerConfig::displayOffKey, defaults.displayOffMinutes);
    idle_group->add_child(new settings_widget("Turn displays off after", "", display_off_spin));
    QCheckBox *dim_check = new QCheckBox;
    binder->bind(dim_check, LockerConfig::dimBeforeDisplayOffKey, defaults.dimBeforeDisplayOff);
    idle_group->add_child(new settings_widget("Dim the screen first", "10 seconds before the displays turn off", dim_check));
    QCheckBox *lock_on_display_off_check = new QCheckBox;
    binder->bind(lock_on_display_off_check, LockerConfig::lockOnDisplayOffKey, defaults.lockOnDisplayOff);
    idle_group->add_child(new settings_widget("Lock when displays turn off", "", lock_on_display_off_check));

    // Dim and lock only apply when the displays turn off at all.
    auto sync_idle_rows = [=]{
        dim_check->setVisible(display_off_spin->value() > 0);
        lock_on_display_off_check->setVisible(display_off_spin->value() > 0);
    };
    connect(display_off_spin, &QSpinBox::valueChanged, this, sync_idle_rows);
    sync_idle_rows();

    settings_widget_group *lock_group = new settings_widget_group("Locking");
    page->add_child(lock_group);
    QCheckBox *lock_on_suspend_check = new QCheckBox;
    binder->bind(lock_on_suspend_check, LockerConfig::lockOnSuspendKey, defaults.lockOnSuspend);
    lock_group->add_child(new settings_widget("Lock on suspend", "", lock_on_suspend_check));
    QSpinBox *locked_display_off_spin = minutesSpin(60);
    binder->bind(locked_display_off_spin, LockerConfig::lockedDisplayOffKey, defaults.lockedDisplayOffMinutes);
    lock_group->add_child(new settings_widget("Turn displays off while locked after", "", locked_display_off_spin));

    return {page};
}
