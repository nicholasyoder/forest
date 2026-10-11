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
#include <QListWidget>
#include <QVBoxLayout>
#include <QMenu>
#include <QDir>
#include <QProcess>
#include <QMimeDatabase>
#include <QFileSystemWatcher>
#include <qt6xdg/XdgDesktopFile>
#include <qt6xdg/XdgMimeType>
#include <qt6xdg/XdgDirs>

#include "wallpaperwidget.h"
#include "iconswidget.h"

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
    Q_SCRIPTABLE void reloadsettings(){ GS::load(); }

private slots:
    void loadwallpaperwidgets();
    void setupmenus();
    void showsettings();
    void updateicons();
    void loadicon(QString file);
    void saveiconlocations(QHash<QString, QString> poshash);
    QRect getusabledesktopspace(QScreen *screen);
    void handleScreenChange();

    void handleiconactivated(QString iconID){openfile(iconID);}
    void paste2desktop(){fileops::paste(desktopdir());}
    void opendesktopfolder(){openfile(desktopdir());}

    void handlekeypressed(QKeyEvent *event);
    void handlefilesdropped(QStringList paths, Qt::DropAction action);
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
    static QString desktopdir(){ return XdgDirs::userDir(XdgDirs::Desktop); }
    static void openfile(const QString &file);
    QStringList selectedpaths();

    bool updatepaused = false;

    QList <wallpaperwidget *> wallwidgetlist;
    iconswidget *iwidget = nullptr;
    QSettings *settings = new QSettings("Forest", "Forest");
    QMenu *deskmenu = new QMenu;
    QMenu *iconmenu = new QMenu;
};
#endif // DESKTOP_H
