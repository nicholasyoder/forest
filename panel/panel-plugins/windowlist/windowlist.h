// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WINDOWLIST_H
#define WINDOWLIST_H

#include <QMainWindow>
#include <QApplication>
#include <QPointer>
#include <QSettings>
#include <QTimer>
#include <QtDBus>
#include <QGenericPlugin>

#include "windowbutton.h"
#include "foreigntoplevelmanager.h"
#include "foreigntoplevelhandle.h"
#include "extforeigntoplevellist.h"
#include "extforeigntoplevelhandle.h"
#include "extworkspacemanager.h"

#include "imagepopup.h"
#include "settingswidget.h"
#include "panelpluginterface.h"

class windowlist : public QWidget, panelpluginterface{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.windowlist.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    windowlist();
    ~windowlist();

    //begin plugin interface
    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){ close(); deleteLater();}
    void XcbEventFilter(xcb_generic_event_t*){}
    QHash<QString, QString> getpluginfo();
    //end plugin interface

public slots:
    void reloadsettings();

signals:
    void changehighlight(xcb_window_t window);
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

    // Per-desktop filtering: only the active workspace's windows are shown
    // (see org.biome.Workspaces.GetWindowWorkspaces - neither
    // wlr-foreign-toplevel-management nor ext-workspace-v1 has any
    // toplevel<->workspace concept on its own). Re-run on every trigger
    // that could change which buttons should be visible: the cached
    // mapping changing (a window opened/closed/moved) or the active
    // workspace itself changing (a desktop switch).
    void refreshWindowWorkspaces();
    void onWindowWorkspacesChanged(QVariantMap windowWorkspaces);
    void refreshVisibility();

private:
    QWidget *stretchwidget = new QWidget;
    QHBoxLayout *mainlayout = new QHBoxLayout();
    QMap<ForeignToplevelHandle*, windowbutton*> button_list;

    ForeignToplevelManager *toplevel_manager = nullptr;
    ExtForeignToplevelList *ext_toplevel_list = nullptr;
    ExtWorkspaceManager *workspace_manager = nullptr;

    // Creation-order pairing between wlr-foreign-toplevel-management and
    // ext-foreign-toplevel-list handles for the same window - see
    // extforeigntoplevellist.h. QPointer, not a raw pointer: an entry here
    // is a handle staged for pairing that hasn't been consumed yet, and
    // Qt's deleteLater()-based deletion means "not consumed yet" and
    // "already destroyed elsewhere" can't be told apart by pointer value
    // alone with a raw QList<T*> - a real use-after-free this way already
    // crashed once (fixed at its root by ExtForeignToplevelHandle's
    // m_readySent guard, see that header), but QPointer auto-nulling on
    // destruction makes tryPairPendingHandles() fail safe instead of
    // dangling if some other bug ever re-introduces a stale entry.
    QList<QPointer<ForeignToplevelHandle>> pending_zwlr_handles;
    QList<QPointer<ExtForeignToplevelHandle>> pending_ext_handles;
    void tryPairPendingHandles();

    // identifier -> workspace index, refreshed from org.biome.Workspaces -
    // see refreshVisibility().
    QVariantMap window_workspaces;

    int maxbtsize;

    popupmenu *pmenu = nullptr;
    imagepopup *ipopup = nullptr;

    settingswidget *swidget = new settingswidget;

};

#endif // WINDOWLIST_H
