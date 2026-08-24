// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WINDOWLIST_H
#define WINDOWLIST_H

#include <QMainWindow>
#include <QApplication>
#include <QSettings>
#include <QTimer>
#include <QtDBus>
#include <QGenericPlugin>

#include "windowbutton.h"
#include "foreigntoplevelmanager.h"
#include "foreigntoplevelhandle.h"

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

    void onButtonMoved(windowbutton *wbt, bool left);
    void onButtonEnter(windowbutton *wbt);
    void onButtonLeave(windowbutton *);

private:
    QWidget *stretchwidget = new QWidget;
    QHBoxLayout *mainlayout = new QHBoxLayout();
    QMap<ForeignToplevelHandle*, windowbutton*> button_list;

    ForeignToplevelManager *toplevel_manager = nullptr;

    int maxbtsize;

    popupmenu *pmenu = nullptr;
    imagepopup *ipopup = nullptr;

    settingswidget *swidget = new settingswidget;

};

#endif // WINDOWLIST_H
