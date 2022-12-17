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

#ifndef PANEL_H
#define PANEL_H

#include <QFrame>
#include <QDebug>
#include <QSettings>
#include <QBoxLayout>
#include <QtDBus>
#include <QtX11Extras>
#include <QtX11Extras/QX11Info>

#include "../../library/fpluginterface/fpluginterface.h"
#include "../library/panelpluginterface.h"

#include "../../library/xcbutills/xcbutills.h"

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

class panel : public QWidget, fpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.plugin")
    Q_INTERFACES(fpluginterface)

public:
    panel();
    ~panel();

    //begin pluginterface
    void setupPlug();
    void XcbEventFilter(xcb_generic_event_t *event);
    QHash<QString, QString> getpluginfo();
    //end pluginterface

public slots:
    void reloadsettings(){loadsettings();}
    void reloadplugins();

    //void movepluginup(int plugnum);
    //void moveplugindown(int plugnum);
    void addplugin(QString path);
    //void addseperator(int plugnum);
    //void setseperatorsettings(int plugnum, int size);
    //void removeplugin(int plugnum);

private slots:
    void showsettings();
    void resetgeometry();

private:
    void loadsettings();
    void loadplugins();

    //put zeroes before single digit numbers
    QString pnum(int number) {if(number<10) return "0"+QString::number(number); else return QString::number(number);}

    int numofstretchplugs = 0;
    QString panelposition;
    int panelsize = 0;
    QSettings *settings = new QSettings("Forest","Panel");
    QBoxLayout *wlayout;
    panelpluginterface *pluginterface;
    QList<panelpluginterface*> xcbpluglist;
    QList<panelpluginterface*> pluglist;
};

#endif // PANEL_H
