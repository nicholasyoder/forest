// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Syncs Forest's theme (light/dark, accent) and icon theme to GTK and
// xdg-desktop-portal-gtk (via GSettings and settings.ini / .gtkrc-2.0).

#pragma once

#include <QColor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QProcess>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <forest-theme/foresttheme.h>

namespace toolkitsync {

namespace detail {

inline QString configHome() {
    const QString env = qEnvironmentVariable("XDG_CONFIG_HOME");
    return env.isEmpty() ? QDir::home().filePath(QStringLiteral(".config")) : env;
}

inline void gsettingsSet(const QString &key, const QString &value) {
    QProcess::startDetached(QStringLiteral("gsettings"),
                            {QStringLiteral("set"), QStringLiteral("org.gnome.desktop.interface"), key, value});
}

// Nearest of GNOME's 9 accent-color enum values (libadwaita's palette).
inline QString nearestGnomeAccent(const QColor &c) {
    if (!c.isValid())
        return QStringLiteral("green");
    struct Named { const char *name; int r, g, b; };
    static constexpr Named palette[] = {
        {"blue",   0x35, 0x84, 0xe4},
        {"teal",   0x21, 0x90, 0xa4},
        {"green",  0x3a, 0x94, 0x4a},
        {"yellow", 0xc8, 0x88, 0x00},
        {"orange", 0xed, 0x5b, 0x00},
        {"red",    0xe6, 0x2d, 0x42},
        {"pink",   0xd5, 0x61, 0x99},
        {"purple", 0x91, 0x41, 0xac},
        {"slate",  0x6f, 0x83, 0x96},
    };
    const int r = c.red(), g = c.green(), b = c.blue();
    const char *best = "green";
    double bestDist = 1e18;
    for (const auto &p : palette) {
        const double rmean = (r + p.r) * 0.5;
        const double dr = r - p.r, dg = g - p.g, db = b - p.b;
        const double dist = (2.0 + rmean / 256.0) * dr * dr
                          + 4.0 * dg * dg
                          + (2.0 + (255.0 - rmean) / 256.0) * db * db;
        if (dist < bestDist) {
            bestDist = dist;
            best = p.name;
        }
    }
    return QString::fromLatin1(best);
}

// Updates or inserts keys in a GTK [Settings] ini file while preserving other lines.
inline void updateGtkIni(const QString &path, const QMap<QString, QString> &updates) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QStringList lines;
    {
        QFile in(path);
        if (in.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream ts(&in);
            while (!ts.atEnd())
                lines.append(ts.readLine());
        }
    }

    QMap<QString, QString> remaining = updates;
    bool inSettings = false;
    int settingsEnd = -1;
    for (int i = 0; i < lines.size(); ++i) {
        const QString trimmed = lines[i].trimmed();
        if (trimmed.startsWith('[')) {
            if (inSettings && settingsEnd < 0)
                settingsEnd = i;
            inSettings = (trimmed == QLatin1String("[Settings]"));
            continue;
        }
        if (!inSettings)
            continue;
        const int eq = lines[i].indexOf('=');
        if (eq <= 0)
            continue;
        const QString key = lines[i].left(eq).trimmed();
        auto it = remaining.find(key);
        if (it != remaining.end()) {
            lines[i] = key + QLatin1Char('=') + it.value();
            remaining.erase(it);
        }
    }

    if (!remaining.isEmpty()) {
        if (inSettings && settingsEnd < 0)
            settingsEnd = lines.size();
        if (settingsEnd < 0) {
            if (!lines.isEmpty() && !lines.last().isEmpty())
                lines.append(QString());
            lines.append(QStringLiteral("[Settings]"));
            settingsEnd = lines.size();
        }
        for (auto it = remaining.constBegin(); it != remaining.constEnd(); ++it)
            lines.insert(settingsEnd++, it.key() + QLatin1Char('=') + it.value());
    }

    QSaveFile out(path);
    if (out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts(&out);
        for (const QString &line : lines)
            ts << line << '\n';
        out.commit();
    }
}

inline void updateGtk2Key(const QString &key, const QString &value) {
    const QString path = QDir::home().filePath(QStringLiteral(".gtkrc-2.0"));
    QStringList lines;
    {
        QFile in(path);
        if (in.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream ts(&in);
            while (!ts.atEnd())
                lines.append(ts.readLine());
        }
    }
    const QString replacement = key + QStringLiteral("=\"") + value + QLatin1Char('"');
    bool replaced = false;
    for (QString &line : lines) {
        const int eq = line.indexOf('=');
        if (eq > 0 && line.left(eq).trimmed() == key) {
            line = replacement;
            replaced = true;
            break;
        }
    }
    if (!replaced)
        lines.append(replacement);

    QSaveFile out(path);
    if (out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts(&out);
        for (const QString &line : lines)
            ts << line << '\n';
        out.commit();
    }
}

} // namespace detail

inline void applyTheme(const QString &themeName) {
    const QString name = themeName.isEmpty() ? foresttheme::defaultTheme() : themeName;
    const QString dir = foresttheme::defaultDir();
    const QStringList layers = foresttheme::chain(dir, name);
    const bool dark = (foresttheme::colorScheme(dir, layers) == Qt::ColorScheme::Dark);
    const QString accent = detail::nearestGnomeAccent(foresttheme::accentColor(dir, layers));

    const QString gtkTheme = dark ? QStringLiteral("Adwaita-dark") : QStringLiteral("Adwaita");
    const QString preferDark = dark ? QStringLiteral("1") : QStringLiteral("0");

    // xdg-desktop-portal-gtk mirrors org.gnome.desktop.interface color-scheme into
    // org.freedesktop.appearance for GTK4/libadwaita, browsers, and Flatpak apps.
    detail::gsettingsSet(QStringLiteral("color-scheme"),
                         dark ? QStringLiteral("prefer-dark") : QStringLiteral("prefer-light"));
    detail::gsettingsSet(QStringLiteral("accent-color"), accent);
    detail::gsettingsSet(QStringLiteral("gtk-theme"), gtkTheme);

    const QString cfg = detail::configHome();
    detail::updateGtkIni(cfg + QStringLiteral("/gtk-3.0/settings.ini"), {
        {QStringLiteral("gtk-theme-name"), gtkTheme},
        {QStringLiteral("gtk-application-prefer-dark-theme"), preferDark},
    });
    detail::updateGtkIni(cfg + QStringLiteral("/gtk-4.0/settings.ini"), {
        {QStringLiteral("gtk-application-prefer-dark-theme"), preferDark},
    });
}

inline void applyIconTheme(const QString &iconTheme) {
    const QString name = iconTheme.isEmpty() ? foresttheme::defaultIconTheme() : iconTheme;
    detail::gsettingsSet(QStringLiteral("icon-theme"), name);

    const QString cfg = detail::configHome();
    detail::updateGtkIni(cfg + QStringLiteral("/gtk-3.0/settings.ini"), {
        {QStringLiteral("gtk-icon-theme-name"), name},
    });
    detail::updateGtkIni(cfg + QStringLiteral("/gtk-4.0/settings.ini"), {
        {QStringLiteral("gtk-icon-theme-name"), name},
    });
    detail::updateGtk2Key(QStringLiteral("gtk-icon-theme-name"), name);
}

inline void apply() {
    QSettings settings(QStringLiteral("Forest"), QStringLiteral("Forest"));
    applyTheme(settings.value(QStringLiteral("theme"), foresttheme::defaultTheme()).toString());
    applyIconTheme(settings.value(QStringLiteral("icon_theme"), foresttheme::defaultIconTheme()).toString());
}

} // namespace toolkitsync
