// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CLOCKCONFIG_H
#define CLOCKCONFIG_H

// QSettings("Forest", "Clock") keys and defaults, shared by the applet and its settings page.
namespace clockconfig {

constexpr char path[] = "desktop/panel/clock"; // must match clock.json's "settings"
constexpr char twelvehour[] = "12hour";
constexpr bool twelvehour_default = true;
constexpr char showseconds[] = "showseconds";
constexpr bool showseconds_default = false;

}

#endif // CLOCKCONFIG_H
