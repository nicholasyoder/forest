// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKTOPCONFIG_H
#define DESKTOPCONFIG_H

#include <QSettings>

#include "wallpaperconfig.h"

// QSettings("Forest", "Forest") group "desktop" keys, shared by the desktop app and its settings page.
namespace desktopconfig {

constexpr char group[] = "desktop";
constexpr char wallpapertype[] = "wallpapertype";           // wallpaper::typeName
constexpr char wallpaper[] = "wallpaper";                   // image path
constexpr char imagemode[] = "imagemode";                   // wallpaper::modeName
constexpr char wallpapercolor[] = "wallpapercolor";         // #rrggbb
constexpr char slideshowdir[] = "slideshowdir";             // directory path
constexpr char slideshowinterval[] = "slideshowinterval";   // seconds
constexpr char slideshowshuffle[] = "slideshowshuffle";
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

// Expects settings to already be in the "desktop" group.
inline wallpaper::Config readWallpaper(const QSettings &settings)
{
    wallpaper::Config cfg;
    cfg.type = wallpaper::typeFromSetting(settings.value(wallpapertype));
    cfg.imagePath = settings.value(wallpaper).toString();
    cfg.directory = settings.value(slideshowdir).toString();
    cfg.mode = wallpaper::modeFromSetting(settings.value(imagemode));
    cfg.color = wallpaper::colorFromSetting(settings.value(wallpapercolor), Qt::black);
    cfg.intervalSeconds = wallpaper::clampInterval(
        settings.value(slideshowinterval, wallpaper::defaultIntervalSeconds).toInt());
    cfg.shuffle = settings.value(slideshowshuffle, false).toBool();
    return cfg;
}

inline void writeWallpaper(QSettings &settings, const wallpaper::Config &cfg)
{
    settings.setValue(wallpapertype, wallpaper::typeName(cfg.type));
    settings.setValue(wallpaper, cfg.imagePath);
    settings.setValue(slideshowdir, cfg.directory);
    settings.setValue(imagemode, wallpaper::modeName(cfg.mode));
    settings.setValue(wallpapercolor, wallpaper::colorName(cfg.color));
    settings.setValue(slideshowinterval, wallpaper::clampInterval(cfg.intervalSeconds));
    settings.setValue(slideshowshuffle, cfg.shuffle);
}

}

#endif // DESKTOPCONFIG_H
