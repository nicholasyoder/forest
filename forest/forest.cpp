/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "forest.h"

forest::forest(){
}

forest::~forest(){
}

void forest::setup(){
    QList<fadewidget*> fwidgetlist;
    foreach (QScreen *screen, qApp->screens()){
        fadewidget *fwidget = new fadewidget(screen);
        fwidgetlist << fwidget;
        fwidget->setGeometry(screen->geometry());
        fwidget->setFixedSize(screen->size());
        fwidget->show();
        fwidget->getimage();
    }

    QDBusConnection::sessionBus().registerService("org.forest");

    if (settings->value("needDefaults", "true").toString() == "true")
        setdefaults();

    loadstylesheet();
    loadplugins();

    QDBusConnection::sessionBus().registerObject("/org/forest", this, QDBusConnection::ExportAllSlots);

    foreach (fadewidget* fwidget, fwidgetlist)
        QTimer::singleShot(1000, fwidget, SLOT(start()));
}

void forest::XcbEventFilter(xcb_generic_event_t *event){
    foreach (fpluginterface *interface, xcbpluglist)
        interface->XcbEventFilter(event);
}

void forest::loadstylesheet(){
    qApp->setStyleSheet(fstyleloader::loadstyle("forest"));
}

void forest::setdefaults(){
    QDir dir("/etc/forest");
    if (dir.exists()) {
        QStringList dirs = dir.entryList();

        QDir destdir(QDir::homePath() + "/.config/Forest");
        if (!destdir.exists()) {
            QDir dir2;
            dir2.mkdir(QDir::homePath() + "/.config/Forest");
        }

        int c = 2;
        while (c < dirs.length()) {
            QProcess cp;
            cp.start("cp /etc/forest/" + dirs.at(c) + " \"" + QDir::homePath() + "/.config/Forest/" + dirs.at(c) + "\"");
            cp.waitForFinished();
            c++;
        }

        settings = new QSettings("Forest","Forest");
        settings->setValue("needDefaults", "false");
    }
    else {
        qDebug() << "forest::setdefaults - No default settings";
    }
}

void forest::loadplugins(){
    settings->beginGroup("plugins");
    foreach(QString key, settings->childGroups()){
        if (!settings->value(key+"/enabled", false).toBool())
            continue;

        QPluginLoader plugloader(settings->value(key+"/path").toString());
        if (plugloader.load()) {
            QObject *plugin = plugloader.instance();
            if (!plugin) continue;

            pluginterface = qobject_cast<fpluginterface *>(plugin);
            if (!pluginterface) continue;

            QHash<QString, QString> info = pluginterface->getpluginfo();
            if (info["needsXcbEvents"] == "true")
                xcbpluglist.append(pluginterface);

            pluginterface->setupPlug();
        }
        else {
            qDebug() << plugloader.errorString();
        }
    }
    settings->endGroup();
}
