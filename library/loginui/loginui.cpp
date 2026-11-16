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
#include <QVBoxLayout>

#include <memory>

#include "loginconfig.h"
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
    if (QFileInfo::exists(loginconfig::wallpaperFile)) {
        QSettings settings(loginconfig::confFile, QSettings::IniFormat);
        m_source = QImage(loginconfig::wallpaperFile);
        m_mode = miscutills::wallpaper_mode_from_setting(settings.value(loginconfig::modeKey));
    }
    if (m_source.isNull()) { // absent or unreadable custom image
        m_source = QImage(loginconfig::defaultWallpaper);
        m_mode = Fill;
    }
}

QImage Wallpaper::scaled(QSize size)
{
    if (m_scaled.size() != size)
        m_scaled = miscutills::get_wallpaper_scaled(m_source, m_mode, size);
    return m_scaled;
}

Card::Card(QWidget *parent)
    : QFrame(parent)
{
    setObjectName("login_Card");
    new QVBoxLayout(this);
    layout()->setSpacing(8);
}

void Card::addWidget(QWidget *widget)
{
    layout()->addWidget(widget);
}

void Card::addLayout(QLayout *layout)
{
    static_cast<QVBoxLayout *>(this->layout())->addLayout(layout);
}

Clock::Clock(QWidget *parent)
    : QLabel(parent)
{
    setObjectName("login_Clock");
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
    if (m_scaled.isNull())
        painter.fillRect(rect(), QColor(30, 30, 30));
    else
        painter.drawImage(rect(), m_scaled);
}

void ScreenBackground::resizeEvent(QResizeEvent *)
{
    m_scaled = m_wallpaper.scaled(size());
}

} // namespace loginui
