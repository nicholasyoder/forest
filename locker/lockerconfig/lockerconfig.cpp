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
    config.displayOffMinutes = minutes(settings, displayOffKey, defaults.displayOffMinutes);
    config.dimBeforeDisplayOff = settings.value(dimBeforeDisplayOffKey, defaults.dimBeforeDisplayOff).toBool();
    config.lockOnDisplayOff = settings.value(lockOnDisplayOffKey, defaults.lockOnDisplayOff).toBool();
    config.lockOnSuspend = settings.value(lockOnSuspendKey, defaults.lockOnSuspend).toBool();
    config.lockedDisplayOffMinutes = minutes(settings, lockedDisplayOffKey, defaults.lockedDisplayOffMinutes);
    return config;
}

bool LockerConfig::operator==(const LockerConfig &other) const
{
    return displayOffMinutes == other.displayOffMinutes && dimBeforeDisplayOff == other.dimBeforeDisplayOff
        && lockOnDisplayOff == other.lockOnDisplayOff && lockOnSuspend == other.lockOnSuspend
        && lockedDisplayOffMinutes == other.lockedDisplayOffMinutes;
}
