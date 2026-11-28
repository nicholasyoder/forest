// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GLOBAL_SETTINGS_H
#define GLOBAL_SETTINGS_H

#include <QSettings>

#include "wallpaperconfig.h"

class GS {

public:
    static wallpaper::Config WALLPAPER_CONFIG;
    static bool SHOW_ICONS;
    static int ICON_SIZE;
    static int GRID_SPACING;
    static bool AUTO_ARRANGE;
    static QString SORT_BY;
    static QString SORT_ORDER;
    static bool FOLDERS_FIRST;
    static bool SHOW_HIDDEN;
    static bool SINGLE_CLICK;

    static void load();
};

#endif // GLOBAL_SETTINGS_H
