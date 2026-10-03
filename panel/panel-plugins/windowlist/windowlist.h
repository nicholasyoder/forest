// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WINDOWLIST_H
#define WINDOWLIST_H

#include <QMainWindow>
#include <QMenu>
#include <QApplication>
#include <QPointer>
#include <QSettings>
#include <QTimer>
#include <QGenericPlugin>

#include "windowbutton.h"
#include "foreigntoplevelmanager.h"
#include "foreigntoplevelhandle.h"
#include "extforeigntoplevellist.h"
#include "extforeigntoplevelhandle.h"
#include "extworkspacemanager.h"
#include "biomeworkspaces.h"

#include "imagepopup.h"
#include "settingswidget.h"
#include "panelpluginterface.h"
#include "panelanchor.h"

class windowlist : public QWidget, panelpluginterface{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.windowlist.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    windowlist();
    ~windowlist();

    //begin plugin interface
    void setupPlug(QBoxLayout *layout, QList<QAction*> itemlist);
    void closePlug(){ close(); deleteLater();}
    QHash<QString, QString> getpluginfo();
    //end plugin interface

public slots:
    void reloadsettings();

signals:
    void updatebuttondata();

protected:
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void loadsettings();
    void showsettingswidget();

    void onWindowAdded(ForeignToplevelHandle *handle);
    void onWindowRemoved(ForeignToplevelHandle *handle);
    void onWindowChanged(ForeignToplevelHandle *handle);

    void onExtToplevelCreated(ExtForeignToplevelHandle *handle);
    void onExtToplevelReady(ExtForeignToplevelHandle *handle);
    void onExtToplevelClosed(ExtForeignToplevelHandle *handle);

    void onButtonMoved(windowbutton *wbt, bool left);
    void onButtonEnter(windowbutton *wbt);
    void onButtonLeave(windowbutton *);

    void refreshVisibility();

private:
    QWidget *stretchwidget = new QWidget;
    QHBoxLayout *mainlayout = new QHBoxLayout();
    QMap<ForeignToplevelHandle*, windowbutton*> button_list;

    ForeignToplevelManager *toplevel_manager = nullptr;
    ExtForeignToplevelList *ext_toplevel_list = nullptr;
    ExtWorkspaceManager *workspace_manager = nullptr;

    // Paired front-to-front; see extforeigntoplevellist.h.
    QList<QPointer<ForeignToplevelHandle>> pending_zwlr_handles;
    QList<QPointer<ExtForeignToplevelHandle>> pending_ext_handles;
    void tryPairPendingHandles();

    BiomeWorkspaces *biome_workspaces = nullptr;

    int maxbtsize;

    QMenu *pmenu = nullptr;
    imagepopup *ipopup = nullptr;

    settingswidget *swidget = new settingswidget;

};

#endif // WINDOWLIST_H
