// SPDX-License-Identifier: LGPL-3.0-or-later

// Root helper run through pkexec by the Login & Lock Wallpaper settings page.
//   set-wallpaper <mode> [<color>]   image on stdin
//   set-mode <mode> [<color>]        keep the installed (or default) image
//   set-solid <color>                solid-colour background
// <mode> is Fill, Fit, Stretch or Center; <color> is #rrggbb (default #000000).
// Takes the image on stdin, never a path, so root never opens a user-chosen file.

#include <QBuffer>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QImageReader>
#include <QSaveFile>
#include <QSettings>

#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>

#include "../../library/loginui/loginconfig.h"

namespace {

constexpr qint64 MaxBytes = 64 << 20;
constexpr qint64 MaxPixels = 40'000'000; // above 8K; also keeps decode under Qt's allocation limit

[[noreturn]] void fail(const char *message)
{
    fprintf(stderr, "forest-login-helper: %s\n", message);
    exit(1);
}

QString parseMode(const QString &arg)
{
    if (!wallpaper::isValidModeName(arg))
        fail("invalid mode");
    return arg;
}

QString parseColor(const QString &arg)
{
    if (!wallpaper::isValidHexColor(arg))
        fail("invalid color");
    return arg.toLower();
}

void writeSettings(const QString &type, const QString &mode, const QString &color)
{
    QDir().mkpath("/etc/forest");
    QSettings settings(loginconfig::confFile, QSettings::IniFormat);
    settings.setValue(loginconfig::typeKey, type);
    if (!mode.isEmpty())
        settings.setValue(loginconfig::modeKey, mode);
    settings.setValue(loginconfig::colorKey, color);
    settings.sync();
    if (settings.status() != QSettings::NoError)
        fail("cannot write Login.conf");
}

void installImage()
{
    QFile in;
    if (!in.open(stdin, QIODevice::ReadOnly))
        fail("cannot read stdin");
    QByteArray data = in.readAll(); // loops to EOF; a single read() stops at the pipe buffer
    if (data.isEmpty() || data.size() > MaxBytes)
        fail("image is empty or too large");

    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer);
    QSize size = reader.size();
    if (!reader.canRead() || !size.isValid() || qint64(size.width()) * size.height() > MaxPixels)
        fail("not a supported image, or too large");
    if (reader.read().isNull()) // truncated/corrupt files fail here, not in the greeter
        fail("cannot decode image");

    if (!QDir().mkpath(loginconfig::wallpaperDir))
        fail("cannot create wallpaper directory");
    QSaveFile out(loginconfig::wallpaperFile);
    if (!out.open(QIODevice::WriteOnly) || out.write(data) != data.size())
        fail("cannot write wallpaper");
    out.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    if (!out.commit())
        fail("cannot write wallpaper");
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    if (geteuid() != 0)
        fail("must run as root");
    umask(022); // greetd's user reads these

    const QStringList args = app.arguments().mid(1);
    const QString cmd = args.value(0);
    if (cmd == "set-wallpaper" && (args.size() == 2 || args.size() == 3)) {
        const QString mode = parseMode(args[1]);
        const QString color = args.size() == 3 ? parseColor(args[2]) : QStringLiteral("#000000");
        installImage();
        writeSettings(wallpaper::typeName(wallpaper::Type::Image), mode, color);
    } else if (cmd == "set-mode" && (args.size() == 2 || args.size() == 3)) {
        const QString mode = parseMode(args[1]);
        const QString color = args.size() == 3 ? parseColor(args[2]) : QStringLiteral("#000000");
        writeSettings(wallpaper::typeName(wallpaper::Type::Image), mode, color);
    } else if (cmd == "set-solid" && args.size() == 2) {
        const QString color = parseColor(args[1]);
        writeSettings(wallpaper::typeName(wallpaper::Type::SolidColor), {}, color);
    } else {
        fail("usage: set-wallpaper <mode> [<color>] | set-mode <mode> [<color>] | set-solid <color>");
    }
    return 0;
}
