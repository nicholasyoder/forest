// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOGINCONFIG_H
#define LOGINCONFIG_H

#include <QFileInfo>
#include <QSettings>

#include "wallpaperconfig.h"

// System-wide login/lock wallpaper: written by forest-login-helper (root), read by loginui and LoginSettings.
namespace loginconfig {

inline constexpr char confFile[] = "/etc/forest/Login.conf";
inline constexpr char wallpaperDir[] = "/var/lib/forest/login";
inline constexpr char wallpaperFile[] = "/var/lib/forest/login/wallpaper"; // absent = default wallpaper
inline constexpr char defaultWallpaper[] = "/usr/share/wallpapers/forest/forest.jpg";
inline constexpr char typeKey[] = "wallpaper/type";        // wallpaper::typeName (Image or SolidColor)
inline constexpr char modeKey[] = "wallpaper/imagemode";   // wallpaper::modeName
inline constexpr char colorKey[] = "wallpaper/color";      // #rrggbb
inline constexpr char helperPath[] = FOREST_LIB_DIR "/forest-login-helper";

inline wallpaper::Config readWallpaper()
{
    QSettings conf(confFile, QSettings::IniFormat);
    wallpaper::Config cfg;
    cfg.type = wallpaper::typeFromSetting(conf.value(typeKey));
    if (cfg.type == wallpaper::Type::Slideshow)
        cfg.type = wallpaper::Type::Image;
    cfg.mode = wallpaper::modeFromSetting(conf.value(modeKey));
    cfg.color = wallpaper::colorFromSetting(conf.value(colorKey), Qt::black);
    cfg.imagePath = QFileInfo::exists(wallpaperFile) ? QString::fromLatin1(wallpaperFile)
                                                     : QString::fromLatin1(defaultWallpaper);
    return cfg;
}

} // namespace loginconfig

#endif // LOGINCONFIG_H
