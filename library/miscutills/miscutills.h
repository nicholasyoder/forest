#ifndef MISCUTILLS_H
#define MISCUTILLS_H

#include <QImage>
#include <QTimer>

enum WALLPAPER_MODE { Fill, Fit, Stretch, Tile, Center};

namespace miscutills {

    // Call forest dbus method, path should start with 'forest/'
    void call_dbus(QString path);

    // Run bash command, wait for finished, and return output
    QString run_shell_command(QString command);

    // Load a custom iconsize directive in a stylesheet
    QSize get_iconsize_stylesheet(QString selector, QString stylesheet);

    // Scale image according to specified mode and target size
    QImage* get_wallpaper_scaled(QImage *source_image, WALLPAPER_MODE mode, QSize target_size);
    QImage* get_wallpaper_scaled(QString wallpaper_file, WALLPAPER_MODE mode, QSize target_size);

    // Pad value with zeros
    QString pad_with_zeros(int number);

    // Create a small 30x20 icon of the specified color
    QIcon make_color_icon(QColor color);

    // Convert QColor to "r,g,b" string
    QString color_to_string(QColor color);

    // Convert "r,g,b" string to QColor
    QColor string_to_color(QString color_string);
};

class RunOnce: public QObject {
    Q_OBJECT
public:
    RunOnce(int delay = 200);
signals:
    void activated();
public slots:
    void try_activate();
private:
    int run_delay;
    QTimer *timer = nullptr;
};

#endif // MISCUTILLS_H
