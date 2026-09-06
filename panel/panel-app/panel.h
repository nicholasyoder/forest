// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PANEL_H
#define PANEL_H

#include <QBoxLayout>
#include <QFrame>
#include <QSettings>
#include <QtDBus>

#include "../../library/pluginutills/app_plugin_interface.h"
#include "../panel-library/panelpluginterface.h"
#include "autohidemanager.h"
#include "geometrymanager.h"

class panelQFrame : public QFrame
{
    Q_OBJECT

public:
    panelQFrame(){}

signals:
    void resized();

protected:
    void resizeEvent(QResizeEvent*){emit resized();}
};

class panel : public QWidget, app_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.app.panel.plugin")
    Q_INTERFACES(app_plugin_interface)

public:
    panel();
    ~panel();

    //begin pluginterface
    void setupPlug();
    void XcbEventFilter(xcb_generic_event_t *event);
    bool needs_xcb_events(){ return true; }
    //end pluginterface

public slots:
    void reloadsettings(){settings->sync(); loadsettings();}
    void reloadplugins();
    void addplugin(QString path);
    void update_panel_size();

private slots:
    void showsettings();

private:
    void loadsettings();
    void loadplugins();

    //put zeroes before single digit numbers
    QString pnum(int number) {if(number<10) return "0"+QString::number(number); else return QString::number(number);}

    int numofstretchplugs = 0;
    QSettings *settings = new QSettings("Forest","Panel");
    QBoxLayout *wlayout;
    panelpluginterface *pluginterface;
    QList<panelpluginterface*> xcbpluglist;
    QList<panelpluginterface*> pluglist;
    AutoHideManager* autohide_manager = nullptr;
    GeometryManager* geometry_manager = nullptr;
    GeometryManager *hp_geometry_manager = nullptr;
    panelQFrame *pframe = nullptr;
};

#endif // PANEL_H
