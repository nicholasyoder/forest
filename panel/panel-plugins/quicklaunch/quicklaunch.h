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

#ifndef QUICKLAUNCH_H
#define QUICKLAUNCH_H

#include <QWidget>
#include <QSettings>
#include <QHBoxLayout>
#include <QGenericPlugin>
#include <QtDBus>

#include "launcher.h"
#include "panelpluginterface.h"

class quicklaunch : public QWidget, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.quicklaunch.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    quicklaunch(QWidget *parent = nullptr);
    ~quicklaunch();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    void XcbEventFilter(xcb_generic_event_t* /*event*/){}
    QHash<QString, QString> getpluginfo();
    //end plugininterface

public slots:
    void reloadlaunchers();
    void addlauncher(QString desktopfilepath);
    void movelauncher(launcher *l, bool up);
    void savelaunchermove();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void loadlaunchers();
    void showpopupmenu(int launchernum);
    void removelauncher();
    QString padwithzeros(int number);

private:
    QList<launcher*> launcherlist;
    QHBoxLayout *basehlayout = new QHBoxLayout;
    QBoxLayout *parentlayout;
    popupmenu *pmenu;
    int currentlauncher = 0;
    QSettings *settings;

};
#endif // QUICKLAUNCH_H
