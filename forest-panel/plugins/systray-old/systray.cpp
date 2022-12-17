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

#include "systray.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xdamage.h>

#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>

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

typedef uchar* UCHARP;

SysTray::SysTray(QWidget* parent): QWidget(parent), m_valid(false), m_trayId(0), m_damageEvent(0), m_damageError(0), m_iconSize(22, 22)
{
}

SysTray::~SysTray()
{
    stopTray();
}

void SysTray::setupPlug(int, QBoxLayout *layout, QList<pmenuitem*>)
{
    basehlayout = new QHBoxLayout(this);
    basehlayout->setSpacing(2);
    basehlayout->setMargin(0);

    // Init the selection later just to ensure that no signals are sent until
    // after construction is done and the creating object has a chance to connect.
    QTimer::singleShot(0, this, SLOT(startTray()));

    layout->addWidget(this);
    basehlayout->setDirection(layout->direction());
}

//x11 event filter - event passed down from forest reimplementation of virtual x11EventFilter function of QApplication
void SysTray::x11EventFilter(XEvent* event)
{
    TrayIcon* icon;

    switch (event->type)
    {
    case ClientMessage:
        clientMessageEvent(&(event->xclient));
        break;

    case DestroyNotify:
        icon = findIcon(event->xany.window);
        if (icon)
        {
            m_iconsList.removeAll(icon);
            delete icon;
            realign();
        }
        break;

    default:
        if (event->type == m_damageEvent + XDamageNotify)
        {
            XDamageNotifyEvent* dmg = reinterpret_cast<XDamageNotifyEvent*>(event);
            icon = findIcon(dmg->drawable);
            if (icon)
                icon->update();
        }
        break;
    }
}

//called from x11EventFilter~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SysTray::clientMessageEvent(XClientMessageEvent* e)
{
    unsigned long opcode;
    opcode = Window(e->data.l[1]);
    Window id;

    switch (opcode)
    {
    case 0:
        id = Window(e->data.l[2]);
        if (id)
            addIcon(id);
        break;

    case 1:
    case 2:
        qDebug() << "we don't show baloons messages.";
        break;

    default:
        if (opcode == atom("_NET_SYSTEM_TRAY_MESSAGE_DATA"))
            qDebug() << "message from dockapp:" << e->data.b;
        break;
    }
}

TrayIcon* SysTray::findIcon(Window id)
{
    foreach(TrayIcon* icon, m_iconsList)
    {
        if (icon->iconId() == id || icon->windowId() == id)
            return icon;
    }
    return nullptr;
}

void SysTray::realign()
{
    foreach(TrayIcon* icon, m_iconsList)
        basehlayout->removeWidget(icon);

    foreach(TrayIcon* icon, m_iconsList)
        basehlayout->addWidget(icon);
}

VisualID SysTray::getVisual()
{
    VisualID visualId = 0;
    Display* dsp = QX11Info::display();

    XVisualInfo templ;
    templ.screen=QX11Info::appScreen();
    templ.depth=32;
    templ.c_class=TrueColor;

    int nvi;
    XVisualInfo* xvi = XGetVisualInfo(dsp, VisualScreenMask|VisualDepthMask|VisualClassMask, &templ, &nvi);

    if (xvi)
    {
        int i;
        XRenderPictFormat* format;
        for (i = 0; i < nvi; i++)
        {
            format = XRenderFindVisualFormat(dsp, xvi[i].visual);
            if (format &&
                    format->type == PictTypeDirect &&
                    format->direct.alphaMask)
            {
                visualId = xvi[i].visualid;
                break;
            }
        }
        XFree(xvi);
    }

    return visualId;
}

//freedesktop systray specification~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SysTray::startTray()
{
    Display* dsp = QX11Info::display();
    Window root = QX11Info::appRootWindow();

    QString s = QString("_NET_SYSTEM_TRAY_S%1").arg(DefaultScreen(dsp));
    Atom _NET_SYSTEM_TRAY_S = atom(s.toAscii());

    if (XGetSelectionOwner(dsp, _NET_SYSTEM_TRAY_S) != None)
    {
        qWarning() << "Another systray is running";
        m_valid = false;
        return;
    }

    // init systray protocol
    m_trayId = XCreateSimpleWindow(dsp, root, -1, -1, 1, 1, 0, 0, 0);

    XSetSelectionOwner(dsp, _NET_SYSTEM_TRAY_S, m_trayId, CurrentTime);
    if (XGetSelectionOwner(dsp, _NET_SYSTEM_TRAY_S) != m_trayId)
    {
        qWarning() << "Can't get systray manager";
        stopTray();
        m_valid = false;
        return;
    }

    int orientation =0;
    XChangeProperty(dsp, m_trayId, atom("_NET_SYSTEM_TRAY_ORIENTATION"), XA_CARDINAL, 32, PropModeReplace, UCHARP(&orientation), 1);

    // ** Visual ********************************
    VisualID visualId = getVisual();
    if (visualId)
        XChangeProperty(QX11Info::display(), m_trayId, atom("_NET_SYSTEM_TRAY_VISUAL"), XA_VISUALID, 32, PropModeReplace, UCHARP(&visualId), 1);
    // ******************************************

    XClientMessageEvent event;
    event.type = ClientMessage;
    event.window = root;
    event.message_type = atom("MANAGER");
    event.format = 32;
    event.data.l[0] = CurrentTime;
    event.data.l[1] = long(_NET_SYSTEM_TRAY_S);
    event.data.l[2] = long(m_trayId);
    event.data.l[3] = 0;
    event.data.l[4] = 0;

    XSendEvent(dsp, root, False, StructureNotifyMask, (XEvent*)&event);
    XDamageQueryExtension(QX11Info::display(), &m_damageEvent, &m_damageError);

    qDebug() << "Systray started";
    m_valid = true;
}

void SysTray::stopTray()
{
    qDeleteAll(m_iconsList);
    if (m_trayId)
    {
        XDestroyWindow(QX11Info::display(), m_trayId);
        m_trayId = 0;
    }
    m_valid = false;
}

void SysTray::addIcon(Window winId)
{
    TrayIcon* icon = new TrayIcon(winId, this);
    if (!icon->isValid())
    {
        delete icon;
        return;
    }

    icon->setIconSize(m_iconSize);
    m_iconsList.append(icon);
    realign();
}

Atom SysTray::atom(const char* atomName)
{
    static QHash<QString, Atom> hash;

    if (hash.contains(atomName))
        return hash.value(atomName);

    Atom atom = XInternAtom(QX11Info::display(), atomName, false);
    hash[atomName] = atom;
    return atom;
}

Q_EXPORT_PLUGIN2(forest-panel-system-tray-plugin, SysTray)
