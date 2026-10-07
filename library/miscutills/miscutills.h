// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MISCUTILLS_H
#define MISCUTILLS_H

#include <QImage>
#include <QPointer>
#include <QTimer>

class QScreen;

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
    bool is_pending() const{return timer.isActive();}
signals:
    void activated();
public slots:
    void try_activate();
private:
    QTimer timer;
};

// Debounced screen-layout watcher. Tracks QPointers, not raw QScreen*, since a
// replacement QScreen can reuse a removed one's address.
class ScreenTracker: public QObject {
    Q_OBJECT
public:
    explicit ScreenTracker(QObject *parent = nullptr);

    // Screen for single-screen surfaces (panel, desktop icons, dialogs): the
    // output named by Forest.conf's display/primary_screen if connected, else the top-left one.
    static QScreen* primary();

signals:
    void screens_replaced(); // added/removed/recreated - surfaces on old screens are gone
    void geometry_changed(); // same screens, new geometry
    void primary_changed(); // same screens, display/primary_screen now names another

private slots:
    void handle_primary_setting();

private:
    void watch(QScreen *screen);
    void handle_change();
    QString primary_name() const;

    QList<QPointer<QScreen>> tracked_screens;
    QString last_primary;
    RunOnce runner{2000};
};

#endif // MISCUTILLS_H
