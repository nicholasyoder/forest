// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GLOBAL_SETTINGS_H
#define GLOBAL_SETTINGS_H

#include <QImage>
#include <QSettings>

#include "miscutills.h"

class GS {

public:
    static QImage WALLPAPER; // decoded only when the path changes
    static QString WALLPAPER_PATH;
    static WALLPAPER_MODE IMAGE_MODE;
    static int ICON_SIZE;

    static void load();
};



#endif // GLOBAL_SETTINGS_H
