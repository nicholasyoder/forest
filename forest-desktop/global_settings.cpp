#include "global_settings.h"

// Initialization / Defaults
QImage * GS::WALLPAPER = nullptr;
ImageMode GS::IMAGE_MODE = Fill;
int GS::ICON_SIZE = 48;

void GS::load(){
    QSettings settings("Forest", "Forest");
    settings.sync();
    WALLPAPER = new QImage(settings.value("desktop/wallpaper").toString());
    IMAGE_MODE = ImageMode(settings.value("desktop/imagemode", Fill).toInt());
    ICON_SIZE = settings.value("desktop/iconsize", 48).toInt();
}
