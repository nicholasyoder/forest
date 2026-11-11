// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FSTYLELOADER_H
#define FSTYLELOADER_H

#include <QString>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QPalette>

class fstyleloader{
public:
    fstyleloader(){}

    // Sets the app palette, then stylesheet, from the current theme.
    // palette_theme reads the palette from that theme instead of the current one.
    // Palette first: QStyleSheetStyle snapshots widget palettes when it polishes.
    static void apply(const QString &classname, const QString &palette_theme = QString()){
        applypalette(palette_theme);
        qApp->setStyleSheet(loadstyle(classname));
    }

    // QSS doesn't reach the app palette, which Qt resolves SVG currentColor
    // (Papirus panel icons) from; unstyled widgets draw from it too. Colours
    // come from the themes' [Palette] group, later layers overriding earlier.
    // (QStyleHints::setColorScheme didn't change the icons here.)
    static void applypalette(const QString &palette_theme = QString()){
        QStringList chain = palette_theme.isEmpty() ? themechain() : QStringList{palette_theme};
        QPalette pal = qApp->palette();
        bool found = false;

        foreach (QString theme, chain) {
            QSettings conf(themesdir() + theme + "/theme.conf", QSettings::IniFormat);
            conf.beginGroup("Palette");
            for (const auto &entry : paletteroles()) {
                if (!conf.contains(entry.key))
                    continue;
                QColor color(conf.value(entry.key).toString());
                if (!color.isValid())
                    continue;
                for (QPalette::ColorRole role : entry.roles)
                    pal.setColor(entry.group, role, color);
                found = true;
            }
        }

        if (found)
            qApp->setPalette(pal);
    }

    static QString loadstyle(QString classname){
        QString themes_dir = themesdir();
        QStringList themes_to_load = themechain();

        QString style = "";
        foreach (QString theme, themes_to_load) {
            QFile file(themes_dir + theme + "/" + classname + ".css");

            if(!file.exists())
                continue;  // Skip missing quietly as this layer may not include css files for all classnames

            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
                qDebug() << file.errorString();
                continue;  // Skip but log the error
            }

            style.append("\n" + file.readAll());
            file.close();
        }

        return style;
    }

private:
    struct PaletteEntry {
        const char *key;
        QPalette::ColorGroup group;
        QList<QPalette::ColorRole> roles;
    };

    // theme.conf [Palette] keys -> roles.
    static const QList<PaletteEntry> &paletteroles(){
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

    static QString themesdir(){ return "/usr/share/forest/themes/"; }

    // parent_themes in load order, then the current theme.
    static QStringList themechain(){
        QSettings settings("Forest", "Forest");
        QString current_theme = settings.value("theme",  "Round-Dark").toString();

        QSettings theme_settings(themesdir() + current_theme + "/theme.conf", QSettings::IniFormat);
        QStringList chain = theme_settings.value("parent_themes").toStringList();
        chain.append(current_theme);
        return chain;
    }
};

#endif // FSTYLELOADER_H
