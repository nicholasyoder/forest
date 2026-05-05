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
#include <qt6xdg/XdgDesktopFile>
#include <qt6xdg/XdgMimeType>
#include <qt6xdg/XdgDirs>

#include "wallpaperwidget.h"
#include "iconswidget.h"

#include "global_settings.h"

#include "../../library/pluginutills/app_plugin_interface.h"
#include "../../library/fmutils/fmutils.h"

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
    void XcbEventFilter(xcb_generic_event_t* /*event*/){}
    bool needs_xcb_events(){ return false; }
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
    void handleScreenChange();

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

    QList <QRect> screen_geos;
    QList <wallpaperwidget *> wallwidgetlist;
    iconswidget *iwidget;
    QSettings *settings = new QSettings("Forest", "Forest");
    QMenu *deskmenu = new QMenu;
    QMenu *iconmenu = new QMenu;
};
#endif // DESKTOP_H
