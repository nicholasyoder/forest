// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKTOPCONFIG_H
#define DESKTOPCONFIG_H

// QSettings("Forest", "Forest") group "desktop" keys, shared by the desktop app and its settings page.
namespace desktopconfig {

constexpr char group[] = "desktop";
constexpr char wallpaper[] = "wallpaper";       // image path
constexpr char imagemode[] = "imagemode";       // name (miscutills::wallpaper_mode_name)
constexpr char showicons[] = "showicons";
constexpr char iconsize[] = "iconsize";         // px
constexpr char gridspacing[] = "gridspacing";   // px between icon cells
constexpr char autoarrange[] = "autoarrange";
constexpr char sortby[] = "sortby";             // name | type | size | modified
constexpr char sortorder[] = "sortorder";       // ascending | descending
constexpr char foldersfirst[] = "foldersfirst";
constexpr char showhidden[] = "showhidden";
constexpr char singleclick[] = "singleclick";

constexpr bool showicons_default = true;
constexpr int iconsize_default = 48;
constexpr int iconsize_min = 24;
constexpr int iconsize_max = 128;
constexpr int gridspacing_default = 4;
constexpr int gridspacing_min = 0;
constexpr int gridspacing_max = 64;
constexpr bool autoarrange_default = false;
constexpr char sortby_default[] = "name";
constexpr char sortorder_default[] = "ascending";
constexpr bool foldersfirst_default = true;
constexpr bool showhidden_default = false;
constexpr bool singleclick_default = false;

}

#endif // DESKTOPCONFIG_H
