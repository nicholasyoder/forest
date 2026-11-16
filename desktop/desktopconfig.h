// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKTOPCONFIG_H
#define DESKTOPCONFIG_H

// QSettings("Forest", "Forest") group "desktop" keys, shared by the desktop app and its settings page.
namespace desktopconfig {

constexpr char group[] = "desktop";
constexpr char wallpaper[] = "wallpaper"; // image path
constexpr char imagemode[] = "imagemode"; // name (miscutills::wallpaper_mode_name)

}

#endif // DESKTOPCONFIG_H
