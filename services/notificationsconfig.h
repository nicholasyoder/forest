// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NOTIFICATIONSCONFIG_H
#define NOTIFICATIONSCONFIG_H

// QSettings("Forest", "Forest") [notifications] keys and defaults, shared by the popups and the settings page.
namespace notificationsconfig {

constexpr char group[] = "notifications";
constexpr char min_timeout[] = "min_timeout"; // seconds
constexpr int min_timeout_default = 3;
constexpr char max_timeout[] = "max_timeout";
constexpr int max_timeout_default = 30;
constexpr char default_timeout[] = "default_timeout";
constexpr int default_timeout_default = 8;
constexpr char height[] = "height"; // fraction of the screen
constexpr double height_default = 0.7;
constexpr char width[] = "width";
constexpr double width_default = 0.5;

}

#endif // NOTIFICATIONSCONFIG_H
