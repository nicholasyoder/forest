// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOCKERCONFIG_H
#define LOCKERCONFIG_H

// forest-locker's settings (QSettings("Forest", "Locker")), shared with its settings plugin.
// Minutes; 0 = never.
struct LockerConfig {
    static constexpr char displayOffKey[] = "display_off_minutes";
    static constexpr char dimBeforeDisplayOffKey[] = "dim_before_display_off";
    static constexpr char lockOnDisplayOffKey[] = "lock_on_display_off";
    static constexpr char lockOnSuspendKey[] = "lock_on_suspend";
    static constexpr char lockedDisplayOffKey[] = "locked_display_off_minutes";

    int displayOffMinutes = 10;
    bool dimBeforeDisplayOff = true;
    bool lockOnDisplayOff = true;
    bool lockOnSuspend = true;
    int lockedDisplayOffMinutes = 1;

    int displayOffMs() const { return displayOffMinutes * 60000; }
    int lockedDisplayOffMs() const { return lockedDisplayOffMinutes * 60000; }

    static LockerConfig load();

    bool operator==(const LockerConfig &other) const;
    bool operator!=(const LockerConfig &other) const { return !(*this == other); }
};

#endif // LOCKERCONFIG_H
