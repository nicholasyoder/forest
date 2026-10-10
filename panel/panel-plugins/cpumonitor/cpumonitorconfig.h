// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CPUMONITORCONFIG_H
#define CPUMONITORCONFIG_H

#include <QColor>

// QSettings("Forest", "CPU Monitor") keys and defaults, shared by the applet and its settings page.
namespace cpumonitorconfig {

constexpr char path[] = "desktop/panel/cpumonitor"; // must match cpumonitor.json's "settings"
constexpr char backgroundcolor[] = "backgroundcolor";
constexpr char backgroundopacity[] = "backgroundopacity";
inline const QColor backgroundcolor_default = Qt::black;
constexpr char foregroundcolor[] = "foregroundcolor";
constexpr char foregroundopacity[] = "foregroundopacity";
inline const QColor foregroundcolor_default = Qt::green;
constexpr char updateinterval[] = "updateinterval";
constexpr int updateinterval_default = 500;
constexpr char command[] = "command";
constexpr char width[] = "width";
constexpr int width_default = 40;

}

#endif // CPUMONITORCONFIG_H
