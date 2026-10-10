// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MEMORYMONITORCONFIG_H
#define MEMORYMONITORCONFIG_H

#include <QColor>

// QSettings("Forest", "Memory Monitor") keys and defaults, shared by the applet and its settings page.
namespace memorymonitorconfig {

constexpr char path[] = "desktop/panel/memorymonitor"; // must match memorymonitor.json's "settings"
constexpr char backgroundcolor[] = "backgroundcolor";
constexpr char backgroundopacity[] = "backgroundopacity";
inline const QColor backgroundcolor_default = Qt::black;
constexpr char ramcolor[] = "RAMcolor";
constexpr char ramopacity[] = "RAMopacity";
inline const QColor ramcolor_default = Qt::red;
constexpr char swapcolor[] = "Swapcolor";
constexpr char swapopacity[] = "Swapopacity";
inline const QColor swapcolor_default = QColor(100, 0, 0);
constexpr char swapbehavior[] = "swapbehavior";
constexpr char swap_disabled[] = "disabled";
constexpr char swap_combine[] = "combine";
constexpr char swap_separate[] = "showseperate";
constexpr char updateinterval[] = "updateinterval";
constexpr int updateinterval_default = 500;
constexpr char command[] = "command";
constexpr char width[] = "width";
constexpr int width_default = 40;

}

#endif // MEMORYMONITORCONFIG_H
