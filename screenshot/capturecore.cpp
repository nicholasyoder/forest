// SPDX-License-Identifier: LGPL-3.0-or-later

#include "capturecore.h"

#include "clipboard.h"

#include <QBuffer>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QScreen>
#include <QStandardPaths>
#include <QTextStream>

QImage capturecore::crop(const QList<Shot> &shots, const QRect &region) {
    qreal scale = 1.0;
    for (const Shot &s : shots)
        if (s.screen->geometry().intersects(region))
            scale = qMax(scale, qreal(s.image.width()) / s.screen->geometry().width());
    QImage out(QSize(qRound(region.width() * scale), qRound(region.height() * scale)),
               QImage::Format_ARGB32_Premultiplied);
    out.fill(Qt::black);
    QPainter p(&out);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    for (const Shot &s : shots) {
        const QRect g = s.screen->geometry();
        const QRect i = g.intersected(region);
        if (i.isEmpty())
            continue;
        const qreal sx = qreal(s.image.width()) / g.width(), sy = qreal(s.image.height()) / g.height();
        const QRectF target((i.x() - region.x()) * scale, (i.y() - region.y()) * scale, i.width() * scale,
                            i.height() * scale);
        const QRectF source((i.x() - g.x()) * sx, (i.y() - g.y()) * sy, i.width() * sx, i.height() * sy);
        p.drawImage(target, s.image, source);
    }
    return out;
}

QImage capturecore::compose(const QList<Shot> &shots) {
    if (shots.size() == 1)
        return shots.first().image;
    QRect bounds;
    for (const Shot &s : shots)
        bounds |= s.screen->geometry();
    return crop(shots, bounds);
}

QString capturecore::autoPath() {
    // xdg-user-dir semantics: PICTURES unset resolves to $HOME.
    QString pictures = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (pictures.isEmpty() || pictures == QDir::homePath())
        pictures = QDir::homePath() + QStringLiteral("/Pictures");
    const QString dir = pictures + QStringLiteral("/Screenshots");
    if (!QDir().mkpath(dir))
        return {};

    const QString base = dir + QStringLiteral("/Screenshot_")
        + QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss"));
    QString file = base + QStringLiteral(".png");
    for (int n = 2; QFileInfo::exists(file); ++n)
        file = QStringLiteral("%1_%2.png").arg(base).arg(n);
    return file;
}

void capturecore::notify(const QString &summary, const QString &body, bool failure) {
    QDBusMessage m = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.Notifications"), QStringLiteral("/org/freedesktop/Notifications"),
        QStringLiteral("org.freedesktop.Notifications"), QStringLiteral("Notify"));
    QVariantMap hints;
    if (failure)
        hints.insert(QStringLiteral("urgency"), QVariant::fromValue(uchar(2)));
    m << QStringLiteral("Screenshot") << uint(0)
      << (failure ? QStringLiteral("dialog-error") : QStringLiteral("camera-photo"))
      << summary << body << QStringList() << hints << -1;
    // Blocking so the message is sent before we exit; short in case no daemon runs.
    QDBusConnection::sessionBus().call(m, QDBus::Block, 1000);
}

int capturecore::deliver(const QImage &image, const Options &opts) {
    QTextStream err(stderr);
    auto fail = [&](const QString &why) {
        err << "forest-screenshot: " << why << Qt::endl;
        notify(QStringLiteral("Screenshot failed"), why, true);
        return 1;
    };

    if (image.isNull())
        return fail(QStringLiteral("Could not capture the screen"));

    QByteArray png;
    QBuffer buf(&png);
    buf.open(QIODevice::WriteOnly);
    if (!image.save(&buf, "PNG"))
        return fail(QStringLiteral("Could not encode the screenshot"));

    QString path;
    if (opts.save) {
        path = opts.outputFile.isEmpty() ? autoPath() : opts.outputFile;
        QFile f(path);
        if (path.isEmpty() || !f.open(QIODevice::WriteOnly) || f.write(png) != png.size())
            return fail(QStringLiteral("Could not save the screenshot"));
    }

    bool copied = true;
    if (opts.clipboard) {
        copied = clipboard::copyPng(png);
        if (!copied)
            err << "forest-screenshot: clipboard copy failed" << Qt::endl;
    }

    QString body = path.isEmpty() ? QStringLiteral("Copied to the clipboard") : path;
    if (!copied)
        body += QStringLiteral("\n(clipboard copy failed)");
    notify(opts.save ? QStringLiteral("Screenshot saved") : QStringLiteral("Screenshot copied"), body, false);
    return 0;
}
