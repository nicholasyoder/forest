// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKTOP_H
#define DESKTOP_H

#include <QWidget>
#include <QPainter>
#include <QtDBus>
#include <QScreen>
#include <QGenericPlugin>
#include <QClipboard>
#include <QApplication>
#include <QSettings>
#include <QMenu>
#include <QDir>
#include <QProcess>
#include <QMimeDatabase>
#include <QFileSystemModel>
#include <qt6xdg/XdgDesktopFile>
#include <qt6xdg/XdgMimeType>
#include <qt6xdg/XdgDirs>

#include "wallpaperwidget.h"
#include "desktopview.h"

#include "global_settings.h"

#include "../../library/pluginutills/app_plugin_interface.h"
#include "fileops.h"

class desktop : public QObject, app_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.app.desktop.plugin")
    Q_INTERFACES(app_plugin_interface)

public:
    desktop();
    ~desktop();

    //begin pluginterface
    void setupPlug();
    //end pluginterface

public slots:
    Q_SCRIPTABLE void reloadwallpaper();
    Q_SCRIPTABLE void nextwallpaper();
    Q_SCRIPTABLE void reloadicons();
    Q_SCRIPTABLE void arrangeicons();

private slots:
    void loadwallpaperwidgets();
    void setupmenus();
    void showwallpapersettings();
    void showiconsettings();
    void syncsortmenu();
    void refresh();
    void handleScreenChange();

    void paste2desktop(){fileops::paste(desktopdir());}
    void opendesktopfolder(){openfile(desktopdir());}

    void handlefilesdropped(QStringList paths, Qt::DropAction action, bool trusted);
    void renamefile(const QString &path, const QString &newname);

    void openselected();
    void cutselected();
    void copyselected();
    void copypathofselected();
    void renameselected(){ if (view) view->renameselected(); }
    void selectall(){ if (view) view->selectAll(); }
    void trashselected();
    void deleteselected();

    void createfolder();
    void createview();
    void createfile();

private:
    static QString desktopdir(){ return XdgDirs::userDir(XdgDirs::Desktop); }
    static desktopview::iconsettings currenticonsettings();
    void updatefilter();
    void savesortsetting(const QString &key, const QVariant &value, bool arrange);
    void openfile(const QString &file);
    QStringList selectedpaths(){ return view ? view->selectedpaths() : QStringList(); }
    QAction *addaction(QMenu *menu, const QString &icon, const QString &text, void (desktop::*slot)(),
                       const QKeySequence &shortcut = {});

    wallpaper::WallpaperSource *wallsource = nullptr;
    QList <wallpaperwidget *> wallwidgetlist;
    QFileSystemModel *fsmodel = nullptr;
    desktopmodel *model = nullptr;
    desktopview *view = nullptr;
    QList<QAction*> viewactions; // keyboard shortcuts while the icon view has focus
    QMenu *deskmenu = new QMenu;
    QMenu *iconmenu = new QMenu;
    QMenu *sortmenu = nullptr;
    QAction *selectallaction = nullptr;
    QAction *foldersfirstaction = nullptr;
    QAction *autoarrangeaction = nullptr;
    QAction *arrangeaction = nullptr;
    QAction *nextwallpaperaction = nullptr;
    QHash<QString, QAction*> sortbyactions;
    QHash<QString, QAction*> sortorderactions;
};
#endif // DESKTOP_H
