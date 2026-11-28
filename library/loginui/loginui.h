// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOGINUI_H
#define LOGINUI_H

#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <QString>

struct passwd;

// UI pieces shared by forest-greeter and forest-lockscreen.
namespace loginui {

// Circular avatar from an image file, or an initial on a grey disc.
QPixmap circularAvatar(const QString &path, int size, const QString &fallback);

// First GECOS field, else the username.
QString displayName(const passwd *pw);

// ~/.face, else the AccountsService icon; empty if neither exists.
QString faceIconPath(const QString &username, const QString &homeDir);

// The themed frame around a login UI.
class Card : public QFrame {
    Q_OBJECT
public:
    explicit Card(QWidget *parent = nullptr);
    void addWidget(QWidget *widget);
    void addLayout(QLayout *layout);
};

class Clock : public QLabel {
    Q_OBJECT
public:
    explicit Clock(QWidget *parent = nullptr);

private:
    void tick();
};

} // namespace loginui

#endif // LOGINUI_H
