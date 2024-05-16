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

#ifndef WINDOWLIST_H
#define WINDOWLIST_H

#include <QMainWindow>
#include <QApplication>
#include <QtX11Extras/QX11Info>
#include <QSettings>
#include <QTimer>
#include <QtDBus>
#include <QGenericPlugin>

#include <KWindowSystem>

#include "windowbutton.h"

//#include "imagepopup.h"
#include "settingswidget.h"
#include "panelpluginterface.h"
#include "xcbutills/xcbutills.h"

class windowlist : public QWidget, panelpluginterface
{
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
    //void closepopups();
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

    void previewWindow();

private:
    QWidget *stretchwidget = new QWidget;
    QHBoxLayout *mainlayout = new QHBoxLayout();
    QList<xcb_window_t> oldwindows;
    QMap<unsigned long, windowbutton*> button_list;

    WId active_window = 0;
    int currentdesk = 0;
    int olddesk = 0;

    QString bttype = "twopart";
    int maxbtsize;

    popupmenu *pmenu = nullptr;
    //imagepopup *ipopup = nullptr;

    settingswidget *swidget = new settingswidget;

};

#endif // WINDOWLIST_H
