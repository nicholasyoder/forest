/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)GPL3+
 *
 * Copyright: 2020 Nicholas Yoder
 * Authors:
 *   Nicholas Yoder <nobody@noemail.com>
 *
 * Modified from elokab systray plugin
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

#ifndef SYSTRAY_H
#define SYSTRAY_H

#include "panelpluginterface.h"

#include <QObject>
#include <QFrame>
#include <QList>
#include <QToolButton>
#include <QtGui>
#include <QApplication>
#include <QtDebug>
#include <QX11Info>
#include <QtCore/QTimer>
#include <QIcon>
#include <QDebug>
#include <QResizeEvent>
#include <QPainter>
#include <QBitmap>

#include "trayicon.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xdamage.h>

class SysTray: public  QWidget, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.systray.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    explicit  SysTray(QWidget* parent = nullptr);
    ~SysTray();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QHash<QString, QVariant> info, QList<pmenuitem*> itemlist);
    void XcbEventFilter(xcb_generic_event_t* event);
    QHash<QString, QString> getpluginfo();
    //end plugininterface

private slots:
    void startTray();
    void stopTray();
    void realign();
    TrayIcon* findIcon(Window trayId);

private:
    VisualID getVisual();
    void clientMessageEvent(XClientMessageEvent* e);
    void addIcon(Window id);
    Atom atom(const char* atomName);

    bool m_valid;
    Window m_trayId;
    QList<TrayIcon*> m_iconsList;
    int m_damageEvent;
    int m_damageError;
    QSize m_iconSize;
    QHBoxLayout *basehlayout;
};
#endif // SYSTRAY_H
