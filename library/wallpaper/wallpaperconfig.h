// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WALLPAPERCONFIG_H
#define WALLPAPERCONFIG_H

#include <QColor>
#include <QDir>
#include <QImageReader>
#include <QString>
#include <QStringList>
#include <QVariant>

// Core/Gui-only wallpaper types and parsing, also used by forest-login-helper without QtWidgets.
namespace wallpaper {

enum class Type { Image, Slideshow, SolidColor };
enum class Mode { Fill, Fit, Stretch, Center };

inline constexpr char defaultWallpaper[] = "/usr/share/wallpapers/forest/forest.jpg";
inline constexpr int minIntervalSeconds = 10;
inline constexpr int maxIntervalSeconds = 86400;
inline constexpr int defaultIntervalSeconds = 900;

struct Config {
    Type type = Type::Image;
    QString imagePath;
    QString directory;
    Mode mode = Mode::Fill;
    QColor color = Qt::black;
    int intervalSeconds = defaultIntervalSeconds;
    bool shuffle = false;

    bool operator==(const Config &other) const
    {
        return type == other.type && imagePath == other.imagePath && directory == other.directory
            && mode == other.mode && color == other.color
            && intervalSeconds == other.intervalSeconds && shuffle == other.shuffle;
    }
    bool operator!=(const Config &other) const { return !(*this == other); }
};

inline QString typeName(Type type)
{
    switch (type) {
    case Type::Slideshow: return QStringLiteral("Slideshow");
    case Type::SolidColor: return QStringLiteral("SolidColor");
    case Type::Image: break;
    }
    return QStringLiteral("Image");
}

inline Type typeFromSetting(const QVariant &value)
{
    const QString text = value.toString().trimmed();
    if (text == QLatin1String("Slideshow")) return Type::Slideshow;
    if (text == QLatin1String("SolidColor")) return Type::SolidColor;
    return Type::Image;
}

inline QString modeName(Mode mode)
{
    switch (mode) {
    case Mode::Fit: return QStringLiteral("Fit");
    case Mode::Stretch: return QStringLiteral("Stretch");
    case Mode::Center: return QStringLiteral("Center");
    case Mode::Fill: break;
    }
    return QStringLiteral("Fill");
}

inline Mode modeFromSetting(const QVariant &value)
{
    const QString text = value.toString().trimmed();
    if (text == QLatin1String("Fit")) return Mode::Fit;
    if (text == QLatin1String("Stretch")) return Mode::Stretch;
    if (text == QLatin1String("Center")) return Mode::Center;
    return Mode::Fill;
}

inline bool isValidModeName(const QString &name)
{
    return name == QLatin1String("Fill") || name == QLatin1String("Fit")
        || name == QLatin1String("Stretch") || name == QLatin1String("Center");
}

inline bool isValidHexColor(const QString &text)
{
    if (text.size() != 7 || text[0] != QLatin1Char('#'))
        return false;
    for (int i = 1; i < 7; ++i) {
        const QChar c = text[i];
        const bool hex = (c >= QLatin1Char('0') && c <= QLatin1Char('9'))
            || (c >= QLatin1Char('a') && c <= QLatin1Char('f'))
            || (c >= QLatin1Char('A') && c <= QLatin1Char('F'));
        if (!hex)
            return false;
    }
    return true;
}

inline QString colorName(const QColor &color)
{
    if (!color.isValid())
        return QStringLiteral("#000000");
    return QColor(color.red(), color.green(), color.blue()).name(QColor::HexRgb);
}

inline QColor colorFromSetting(const QVariant &value, const QColor &fallback = Qt::black)
{
    QColor c = QColor::fromString(value.toString().trimmed());
    if (!c.isValid())
        return fallback;
    c.setAlpha(255);
    return c;
}

inline int clampInterval(int seconds)
{
    return qBound(minIntervalSeconds, seconds, maxIntervalSeconds);
}

inline QStringList imageNameFilters()
{
    QStringList patterns;
    for (const QByteArray &format : QImageReader::supportedImageFormats())
        patterns << QStringLiteral("*.") + QString::fromLatin1(format);
    return patterns;
}

inline QStringList listImagesInDirectory(const QString &dirPath)
{
    if (dirPath.isEmpty())
        return {};
    QDir dir(dirPath);
    if (!dir.exists())
        return {};
    const QStringList entries = dir.entryList(imageNameFilters(), QDir::Files | QDir::Readable,
                                              QDir::Name | QDir::IgnoreCase);
    QStringList result;
    result.reserve(entries.size());
    for (const QString &entry : entries)
        result << dir.filePath(entry);
    return result;
}

} // namespace wallpaper

#endif // WALLPAPERCONFIG_H
