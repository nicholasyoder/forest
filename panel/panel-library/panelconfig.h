// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PANELCONFIG_H
#define PANELCONFIG_H

// QSettings("Forest", "Panel") keys and defaults, shared by the panel and its settings page.
namespace panelconfig {

constexpr char position[] = "position"; // "Top" or "Bottom"; compare lowercased
constexpr char position_default[] = "Bottom";
constexpr char autohide[] = "autohide";
constexpr bool autohide_default = false;
constexpr char autohide_delay[] = "autohide_delay"; // ms
constexpr int autohide_delay_default = 1000;

}

#endif // PANELCONFIG_H
