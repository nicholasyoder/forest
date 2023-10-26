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

#ifndef DESKTOP_H
#define DESKTOP_H

#include <QWidget>
#include <QPainter>
#include <QtDBus>
#include <QScreen>
#include <QGenericPlugin>
#include <QDesktopWidget>
#include <QClipboard>
#include <QApplication>
#include <QSettings>
#include <QListWidget>
#include <QVBoxLayout>
#include <QMenu>
#include <qt5xdg/XdgDesktopFile>
#include <qt5xdg/XdgMimeType>
#include <qt5xdg/XdgDirs>

#include "wallpaperwidget.h"
#include "iconswidget.h"

#include "global_settings.h"

#include "../library/fpluginterface/fpluginterface.h"
#include "../library/fmutils/fmutils.h"
#include "../library/fstyleloader/fstyleloader.h"

class desktop : public QObject, fpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.desktop.plugin")
    Q_INTERFACES(fpluginterface)

public:
    desktop();
    ~desktop();

    //begin pluginterface
    void setupPlug();
    void XcbEventFilter(xcb_generic_event_t* /*event*/){}
    fpluginfo getpluginfo();
    //end pluginterface

public slots:
    Q_SCRIPTABLE void reloadwallpaper();
    Q_SCRIPTABLE void reloadsettings(){ GS::load(); }

private slots:
    void loadwallpaperwidgets();
    void setupmenus();
    void showsettings();
    void updateicons();
    void loadicon(QString file);
    void saveiconlocations(QHash<QString, QString> poshash);
    QRect getusabledesktopspace();
    void handleAvailableGeoChange(QRect geo);

    void handleiconactivated(QString iconID){fmutils::openfile(iconID);}
    void paste2desktop(){fmutils::pastefromclipboard(QDir::homePath() + "/Desktop");}
    void opendesktopfolder(){fmutils::openfile(QDir::homePath() + "/Desktop");}

    void handlekeypressed(QKeyEvent *event);
    void handlekeyreleased(QKeyEvent *event);
    void handlefilesdropped(QList<QUrl> urls);
    void handleicontextchanged(QString ID, QString newtext);

    void openselected();
    void cutselected();
    void copyselected();
    void copypathofselected();
    void renameselected();
    void trashselected();
    void deleteselected();

    void createfolder();
    void createfile();

private:
    bool ctrldown = false;
    bool shiftdown = false;
    bool updatepaused = false;

    QList <wallpaperwidget *> wallwidgetlist;
    iconswidget *iwidget;
    QSettings *settings = new QSettings("Forest", "Forest");
    QMenu *deskmenu = new QMenu;
    QMenu *iconmenu = new QMenu;
};
#endif // DESKTOP_H
