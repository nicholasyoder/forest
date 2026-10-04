// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOGINUI_H
#define LOGINUI_H

#include <QHash>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QWidget>

// UI pieces shared by forest-greeter and forest-lockscreen.
namespace loginui {

// Circular avatar from an image file, or an initial on a grey disc.
QPixmap circularAvatar(const QString &path, int size, const QString &fallback);

// ~/.face, else the AccountsService icon; empty if neither exists.
QString faceIconPath(const QString &username, const QString &homeDir);

// The user's desktop wallpaper, scaled per target size (cached).
class Wallpaper {
public:
    Wallpaper();
    QImage scaled(QSize size);

private:
    QString m_file;
    int m_mode;
    QHash<quint64, QImage> m_cache;
};

class Clock : public QLabel {
    Q_OBJECT
public:
    explicit Clock(QWidget *parent = nullptr);

private:
    void tick();
};

// Wallpaper filling the whole widget; meant as one per QScreen.
class ScreenBackground : public QWidget {
    Q_OBJECT
public:
    explicit ScreenBackground(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Wallpaper m_wallpaper;
};

} // namespace loginui

#endif // LOGINUI_H
