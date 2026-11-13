// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FSTYLELOADER_H
#define FSTYLELOADER_H

#include <QString>
#include <QSettings>
#include <QApplication>
#include <QPalette>
#include <forest-theme/foresttheme.h>
#include "forestoutlinestyle.h"

class fstyleloader{
public:
    fstyleloader(){}

    // Sets the app palette, then stylesheet, from the current theme.
    // palette_theme reads the palette from that theme instead of the current one.
    // Palette first: QStyleSheetStyle snapshots widget palettes when it polishes.
    static void apply(const QString &classname, const QString &palette_theme = QString()){
        static bool style_set = false;
        if (!style_set) {
            qApp->setStyle(new ForestOutlineStyle); // resets the palette, so before applypalette
            style_set = true;
        }
        applypalette(palette_theme);
        // shared.css is styling common to Forest and Biome (e.g. QMenu).
        qApp->setStyleSheet(foresttheme::stylesheet(themesdir(), themechain(), {classname, "shared"}));
    }

    // QSS doesn't reach the app palette, which Qt resolves SVG currentColor
    // (Papirus panel icons) from; unstyled widgets draw from it too.
    // (QStyleHints::setColorScheme didn't change the icons here.)
    static void applypalette(const QString &palette_theme = QString()){
        QStringList layers = palette_theme.isEmpty() ? themechain() : QStringList{palette_theme};
        bool found = false;
        QPalette pal = foresttheme::palette(themesdir(), layers, qApp->palette(), &found);
        if (found)
            qApp->setPalette(pal);
    }

    static QString loadstyle(QString classname){
        return foresttheme::stylesheet(themesdir(), themechain(), {classname});
    }

private:
    static QString themesdir(){ return foresttheme::defaultDir(); }

    static QStringList themechain(){
        QSettings settings("Forest", "Forest");
        return foresttheme::chain(themesdir(), settings.value("theme", "Round-Dark").toString());
    }
};

#endif // FSTYLELOADER_H
