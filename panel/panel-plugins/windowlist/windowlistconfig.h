// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WINDOWLISTCONFIG_H
#define WINDOWLISTCONFIG_H

// QSettings("Forest", "Window List") keys and defaults, shared by the applet and its settings page.
namespace windowlistconfig {

constexpr char path[] = "desktop/panel/windowlist"; // must match windowlist.json's "settings"
constexpr char showthumbnails[] = "showthumbnails";
constexpr bool showthumbnails_default = true;
constexpr char maxbuttonsize[] = "maxbuttonsize";
constexpr int maxbuttonsize_default = 170;

}

#endif // WINDOWLISTCONFIG_H
