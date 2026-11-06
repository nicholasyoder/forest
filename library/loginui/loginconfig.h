// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOGINCONFIG_H
#define LOGINCONFIG_H

// System-wide login/lock wallpaper: written by forest-login-helper (root), read by loginui::Wallpaper.
namespace loginconfig {

inline constexpr char confFile[] = "/etc/forest/Login.conf";
inline constexpr char wallpaperDir[] = "/var/lib/forest/login";
inline constexpr char wallpaperFile[] = "/var/lib/forest/login/wallpaper"; // absent = default wallpaper
inline constexpr char defaultWallpaper[] = "/usr/share/wallpapers/forest/forest.jpg";
inline constexpr char modeKey[] = "wallpaper/imagemode"; // miscutills WALLPAPER_MODE
inline constexpr char helperPath[] = "/usr/lib/forest/forest-login-helper";

} // namespace loginconfig

#endif // LOGINCONFIG_H
