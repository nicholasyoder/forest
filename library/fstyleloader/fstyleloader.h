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

    // Sets the app stylesheet and palette from the current theme.
    // forced_scheme overrides the theme's color_scheme.
    static void apply(const QString &classname, const QString &forced_scheme = QString()){
        qApp->setStyleSheet(loadstyle(classname));
        applypalette(forced_scheme);
    }

    // QSS doesn't reach the app palette, which Qt resolves SVG currentColor
    // (Papirus panel icons) from. Last `color_scheme` in the chain wins.
    static void applypalette(const QString &forced_scheme = QString()){
        QString scheme = forced_scheme;
        if (scheme.isEmpty()) {
            foreach (QString theme, themechain())
                scheme = QSettings(themesdir() + theme + "/theme.conf", QSettings::IniFormat)
                             .value("color_scheme", scheme).toString();
        }
        if (scheme != "dark" && scheme != "light")
            return;

        QColor text = scheme == "dark" ? QColor(223, 223, 223) : QColor(40, 40, 40);
        QPalette pal = qApp->palette();
        for (QPalette::ColorRole role : {QPalette::WindowText, QPalette::Text, QPalette::ButtonText})
            pal.setColor(role, text);
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
