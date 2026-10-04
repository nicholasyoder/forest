// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lockerconfig.h"

#include <QSettings>

namespace {

// Hand-edited values: keep the ms conversion in int range.
int minutes(const QSettings &settings, const char *key, int fallback)
{
    return qBound(0, settings.value(key, fallback).toInt(), 24 * 60);
}

} // namespace

LockerConfig LockerConfig::load()
{
    QSettings settings("Forest", "Locker");
    LockerConfig defaults;
    LockerConfig config;
    config.displayOffMinutes = minutes(settings, "display_off_minutes", defaults.displayOffMinutes);
    config.dimBeforeDisplayOff = settings.value("dim_before_display_off", defaults.dimBeforeDisplayOff).toBool();
    config.lockOnDisplayOff = settings.value("lock_on_display_off", defaults.lockOnDisplayOff).toBool();
    config.lockOnSuspend = settings.value("lock_on_suspend", defaults.lockOnSuspend).toBool();
    config.lockedDisplayOffMinutes = minutes(settings, "locked_display_off_minutes", defaults.lockedDisplayOffMinutes);
    return config;
}

void LockerConfig::save() const
{
    QSettings settings("Forest", "Locker");
    settings.setValue("display_off_minutes", displayOffMinutes);
    settings.setValue("dim_before_display_off", dimBeforeDisplayOff);
    settings.setValue("lock_on_display_off", lockOnDisplayOff);
    settings.setValue("lock_on_suspend", lockOnSuspend);
    settings.setValue("locked_display_off_minutes", lockedDisplayOffMinutes);
}

bool LockerConfig::operator==(const LockerConfig &other) const
{
    return displayOffMinutes == other.displayOffMinutes && dimBeforeDisplayOff == other.dimBeforeDisplayOff
        && lockOnDisplayOff == other.lockOnDisplayOff && lockOnSuspend == other.lockOnSuspend
        && lockedDisplayOffMinutes == other.lockedDisplayOffMinutes;
}
