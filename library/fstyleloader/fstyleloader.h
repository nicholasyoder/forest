// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FSTYLELOADER_H
#define FSTYLELOADER_H

#include <QString>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QDebug>

class fstyleloader{
public:
    fstyleloader(){}

    static QString loadstyle(QString classname){
        QString themes_dir =  "/usr/share/forest/themes/";

        QSettings settings("Forest", "Forest");
        QString current_theme = settings.value("theme",  "Round-Dark").toString();

        QSettings theme_settings(themes_dir + current_theme + "/theme.conf", QSettings::IniFormat);
        QStringList themes_to_load = theme_settings.value("parent_themes").toStringList();
        themes_to_load.append(current_theme);

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
};

#endif // FSTYLELOADER_H
