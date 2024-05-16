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
#include "fadewidget.h"
#include "pluginutills.h"
#include "../library/fstyleloader/fstyleloader.h"

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
    foreach (app_plugin_interface *interface, xcbpluglist)
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
    QStringList plugin_paths = pluginutills::get_plugin_paths(APP_PLUGIN);
    foreach (QString plugin_path, plugin_paths) {
        QPluginLoader plugloader(plugin_path);
        if (plugloader.load()) {
            QObject *plugin = plugloader.instance();
            if (!plugin) continue;

            app_plugin_interface *pluginterface = qobject_cast<app_plugin_interface *>(plugin);
            if (!pluginterface) continue;

            if (pluginterface->needs_xcb_events())
                xcbpluglist.append(pluginterface);

            pluginterface->setupPlug();
        }
        else {
            qDebug() << plugloader.errorString();
        }
    }
}
