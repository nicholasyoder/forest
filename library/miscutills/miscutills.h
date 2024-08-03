#ifndef MISCUTILLS_H
#define MISCUTILLS_H

#include <QImage>

enum WALLPAPER_MODE { Fill, Fit, Stretch, Tile, Center};

class miscutills
{
public:
    miscutills();

    // Call forest dbus method, path should start with 'forest/'
    static void call_dbus(QString path);

    // Run bash command, wait for finished, and return output
    static QString run_shell_command(QString command);

    // Load a custom iconsize directive in a stylesheet
    static QSize get_iconsize_stylesheet(QString selector, QString stylesheet);

    // Scale image according to specified mode and target size
    static QImage* get_wallpaper_scaled(QImage *source_image, WALLPAPER_MODE mode, QSize target_size);
    static QImage* get_wallpaper_scaled(QString wallpaper_file, WALLPAPER_MODE mode, QSize target_size);
};

#endif // MISCUTILLS_H
