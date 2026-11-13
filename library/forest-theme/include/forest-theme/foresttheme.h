// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Forest theme format, shared by Forest and Biome: layer chain (theme.conf
// parent_themes), [Palette] -> QPalette, and per-layer CSS concatenation.
// Header-only; Qt Core + Gui only.

#pragma once

#include <QColor>
#include <QDir>
#include <QFile>
#include <QList>
#include <QPalette>
#include <QSettings>
#include <QString>
#include <QStringList>

namespace foresttheme {

inline QString defaultDir() { return QStringLiteral("/usr/share/forest/themes"); }

// parent_themes in load order, then `name` itself.
inline QStringList chain(const QString &dir, const QString &name) {
    QSettings conf(QDir(dir).filePath(name + "/theme.conf"), QSettings::IniFormat);
    QStringList layers = conf.value("parent_themes").toStringList();
    layers.append(name);
    return layers;
}

// Concatenates `<layer>/<file>.css` over the layers, for each file in turn;
// missing files are skipped.
inline QString stylesheet(const QString &dir, const QStringList &layers, const QStringList &files) {
    QString sheet;
    for (const QString &file : files) {
        for (const QString &layer : layers) {
            QFile f(QDir(dir).filePath(layer + "/" + file + ".css"));
            if (f.open(QIODevice::ReadOnly | QIODevice::Text))
                sheet += "\n" + QString::fromUtf8(f.readAll());
        }
    }
    return sheet;
}

namespace detail {

struct PaletteEntry {
    const char *key;
    QPalette::ColorGroup group;
    QList<QPalette::ColorRole> roles;
};

// theme.conf [Palette] keys -> roles.
inline const QList<PaletteEntry> &paletteEntries() {
    static const QList<PaletteEntry> entries = {
        {"window", QPalette::All, {QPalette::Window}},
        {"base", QPalette::All, {QPalette::Base}},
        {"alternate_base", QPalette::All, {QPalette::AlternateBase}},
        {"button", QPalette::All, {QPalette::Button}},
        {"tooltip_base", QPalette::All, {QPalette::ToolTipBase}},
        {"text", QPalette::All, {QPalette::WindowText, QPalette::Text, QPalette::ButtonText, QPalette::ToolTipText}},
        {"disabled_text", QPalette::Disabled, {QPalette::WindowText, QPalette::Text, QPalette::ButtonText, QPalette::ToolTipText}},
        {"placeholder_text", QPalette::All, {QPalette::PlaceholderText}},
        {"highlight", QPalette::All, {QPalette::Highlight}},
        {"highlighted_text", QPalette::All, {QPalette::HighlightedText}},
        {"link", QPalette::All, {QPalette::Link}},
        {"light", QPalette::All, {QPalette::Light}},
        {"midlight", QPalette::All, {QPalette::Midlight}},
        {"mid", QPalette::All, {QPalette::Mid}},
        {"dark", QPalette::All, {QPalette::Dark}},
        {"shadow", QPalette::All, {QPalette::Shadow}},
    };
    return entries;
}

} // namespace detail

// `base` with each layer's [Palette] applied in order, later layers winning.
// *found is set if any color was applied.
inline QPalette palette(const QString &dir, const QStringList &layers, const QPalette &base, bool *found = nullptr) {
    QPalette pal = base;
    bool any = false;
    for (const QString &layer : layers) {
        QSettings conf(QDir(dir).filePath(layer + "/theme.conf"), QSettings::IniFormat);
        conf.beginGroup("Palette");
        for (const auto &entry : detail::paletteEntries()) {
            if (!conf.contains(entry.key))
                continue;
            const QColor color(conf.value(entry.key).toString());
            if (!color.isValid())
                continue;
            for (QPalette::ColorRole role : entry.roles)
                pal.setColor(entry.group, role, color);
            any = true;
        }
    }
    if (found)
        *found = any;
    return pal;
}

} // namespace foresttheme
