// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VOLUMECONFIG_H
#define VOLUMECONFIG_H

#include <QString>

// QSettings("Forest", "Volume Manager") keys and defaults, shared by the applet and its settings page.
// Devices are identified by AudioDevice::description().
namespace volumeconfig {

constexpr char path[] = "desktop/panel/volume"; // must match volume.json's "settings"
constexpr char master[] = "master"; // first device if unset or missing
constexpr char autosave[] = "autosave";
constexpr bool autosave_default = true;
constexpr bool show_default = true;
inline QString show(const QString &device){ return device + "/show"; }
inline QString volume(const QString &device){ return device + "/volume"; }

}

#endif // VOLUMECONFIG_H
