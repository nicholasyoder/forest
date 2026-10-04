// SPDX-License-Identifier: LGPL-3.0-or-later

#include "loginui.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QSettings>
#include <QTextStream>
#include <QTimer>

#include <memory>

#include "miscutills.h"

namespace loginui {

QPixmap circularAvatar(const QString &path, int size, const QString &fallback)
{
    QPixmap result(size, size);
    result.fill(Qt::transparent);
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);

    QPixmap src;
    if (!path.isEmpty())
        src.load(path);

    if (!src.isNull()) {
        src = src.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPainterPath clip;
        clip.addEllipse(0, 0, size, size);
        p.setClipPath(clip);
        p.drawPixmap((size - src.width()) / 2, (size - src.height()) / 2, src);
    } else {
        p.setBrush(QColor(90, 90, 90));
        p.setPen(Qt::NoPen);
        p.drawEllipse(0, 0, size, size);
        p.setPen(Qt::white);
        QFont f;
        f.setPixelSize(size / 2);
        f.setBold(true);
        p.setFont(f);
        QString ch = fallback.isEmpty() ? "?" : fallback.left(1).toUpper();
        p.drawText(QRect(0, 0, size, size), Qt::AlignCenter, ch);
    }
    return result;
}

QString faceIconPath(const QString &username, const QString &homeDir)
{
    QString facePath = homeDir + "/.face";
    if (QFileInfo::exists(facePath))
        return facePath;

    QFile accountsFile("/var/lib/AccountsService/users/" + username);
    if (accountsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream as(&accountsFile);
        while (!as.atEnd()) {
            QString l = as.readLine();
            if (l.startsWith("Icon="))
                return l.mid(5).trimmed();
        }
    }
    return {};
}

Wallpaper::Wallpaper()
{
    QSettings settings("Forest", "Forest");
    m_file = settings.value("desktop/wallpaper", "/usr/share/wallpapers/forest/forest.jpg").toString();
    m_mode = settings.value("desktop/imagemode", Fill).toInt();
}

QImage Wallpaper::scaled(QSize size)
{
    if (m_file.isEmpty() || size.isEmpty())
        return {};
    quint64 key = (quint64(size.width()) << 32) | quint32(size.height());
    auto it = m_cache.constFind(key);
    if (it != m_cache.constEnd())
        return *it;

    std::unique_ptr<QImage> img(miscutills::get_wallpaper_scaled(m_file, WALLPAPER_MODE(m_mode), size));
    QImage result = img ? *img : QImage();
    m_cache.insert(key, result);
    return result;
}

Clock::Clock(QWidget *parent)
    : QLabel(parent)
{
    setObjectName("greeter_Clock");
    setAlignment(Qt::AlignCenter);
    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Clock::tick);
    timer->start(1000);
    tick();
}

void Clock::tick()
{
    setText(QDateTime::currentDateTime().toString("hh:mm"));
}

ScreenBackground::ScreenBackground(QWidget *parent)
    : QWidget(parent)
{
}

void ScreenBackground::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QImage img = m_wallpaper.scaled(size());
    if (img.isNull())
        painter.fillRect(rect(), QColor(30, 30, 30));
    else
        painter.drawImage(rect(), img);
}

} // namespace loginui
