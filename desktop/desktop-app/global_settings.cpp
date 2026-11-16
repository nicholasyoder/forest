// SPDX-License-Identifier: LGPL-3.0-or-later

#include "global_settings.h"

// Initialization / Defaults
QImage GS::WALLPAPER;
QString GS::WALLPAPER_PATH;
WALLPAPER_MODE GS::IMAGE_MODE = Fill;
int GS::ICON_SIZE = 48;

void GS::load(){
    QSettings settings("Forest", "Forest");
    settings.sync();
    const QString path = settings.value("desktop/wallpaper").toString();
    if (path != WALLPAPER_PATH || WALLPAPER.isNull()){
        WALLPAPER_PATH = path;
        WALLPAPER = QImage(path);
    }
    IMAGE_MODE = miscutills::wallpaper_mode_from_setting(settings.value("desktop/imagemode"));
    ICON_SIZE = settings.value("desktop/iconsize", 48).toInt();
}
