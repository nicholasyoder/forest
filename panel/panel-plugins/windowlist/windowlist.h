// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WINDOWLIST_H
#define WINDOWLIST_H

#include <QMainWindow>
#include <QApplication>
#include <QSettings>
#include <QTimer>
#include <QtDBus>
#include <QGenericPlugin>

#include <KWindowSystem>
#include <kx11extras.h>

#include "windowbutton.h"

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

    bool acceptWindow(WId window) const;

    void onWindowAdded(WId window);
    void onWindowRemoved(WId window);
    void onWindowChanged(WId window, NET::Properties prop, NET::Properties2 prop2);
    void onDesktopChanged(int desktop);

    void onButtonMoved(windowbutton *wbt, bool left);
    void onButtonEnter(windowbutton *wbt);
    void onButtonLeave(windowbutton *);

private:
    QWidget *stretchwidget = new QWidget;
    QHBoxLayout *mainlayout = new QHBoxLayout();
    QList<xcb_window_t> oldwindows;
    QMap<unsigned long, windowbutton*> button_list;

    WId active_window = 0;
    int currentdesk = 0;
    int olddesk = 0;

    int maxbtsize;

    popupmenu *pmenu = nullptr;
    imagepopup *ipopup = nullptr;

    settingswidget *swidget = new settingswidget;

};

#endif // WINDOWLIST_H
