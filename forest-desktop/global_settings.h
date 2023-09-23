#ifndef GLOBAL_SETTINGS_H
#define GLOBAL_SETTINGS_H

#include <QImage>
#include <QSettings>

enum ImageMode { Fill, Fit, Stretch, Tile, Center};

class GS {

public:
    static QImage * WALLPAPER;
    static ImageMode IMAGE_MODE;
    static int ICON_SIZE;

    static void load();
};



#endif // GLOBAL_SETTINGS_H
