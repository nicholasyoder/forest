// SPDX-License-Identifier: LGPL-3.0-or-later

// Root helper run through pkexec by the Login Screen settings page.
//   set-wallpaper <mode>   image on stdin
//   set-mode <mode>        keep the installed image
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

int parseMode(const QString &arg)
{
    bool ok = false;
    int mode = arg.toInt(&ok);
    if (!ok || mode < 0 || mode > 4) // miscutills WALLPAPER_MODE
        fail("invalid mode");
    return mode;
}

void writeMode(int mode)
{
    QDir().mkpath("/etc/forest");
    QSettings settings(loginconfig::confFile, QSettings::IniFormat);
    settings.setValue(loginconfig::modeKey, mode);
    settings.sync();
    if (settings.status() != QSettings::NoError)
        fail("cannot write Login.conf");
}

void installImage()
{
    QFile in;
    in.open(stdin, QIODevice::ReadOnly);
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
    if (args.value(0) == "set-wallpaper" && args.size() == 2) {
        int mode = parseMode(args[1]);
        installImage();
        writeMode(mode);
    } else if (args.value(0) == "set-mode" && args.size() == 2) {
        if (!QFile::exists(loginconfig::wallpaperFile))
            fail("no custom wallpaper installed");
        writeMode(parseMode(args[1]));
    } else {
        fail("usage: set-wallpaper <mode> | set-mode <mode>");
    }
    return 0;
}
