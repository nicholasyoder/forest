#ifndef FSTYLELOADER_H
#define FSTYLELOADER_H

#include <QString>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QDebug>

class fstyleloader
{
public:
    fstyleloader(){}

    static QString loadstyle(QString classname)
    {
        QSettings settings("Forest", "Forest");

        QFile file("/usr/share/forest/themes/" + settings.value("theme",  "Round-Dark").toString() + "/" + classname + ".css");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            qDebug() << file.errorString();
            return defaultstyle();
        }

        QString style = file.readAll();
        file.close();
        return style;
    }

private:
    static QString defaultstyle() { return "";}
};

#endif // FSTYLELOADER_H
