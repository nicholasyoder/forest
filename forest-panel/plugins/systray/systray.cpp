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

#include "systray.h"

#include <xcb/damage.h>
#include <xcb/render.h>

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2
#define XEMBED_EMBEDDED_NOTIFY  0
#define XEMBED_MAPPED          (1 << 0)

systray::systray()
{
}

systray::~systray()
{
    stoptray();
}

void systray::setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist)
{
    Q_UNUSED(itemlist);

    mDisplay = QX11Info::display();
    _NET_SYSTEM_TRAY_OPCODE = Xcbutills::atom("_NET_SYSTEM_TRAY_OPCODE");

    traylayout = new QHBoxLayout(this);
    traylayout->setMargin(0);
    traylayout->setSpacing(0);
    layout->addWidget(this);

    iconsize = QSize(22,22);
    QTimer::singleShot(0, this, &systray::starttray);
}

void systray::XcbEventFilter(xcb_generic_event_t *event)
{
    TrayIcon* icon;
    int event_type = event->response_type & ~0x80;

    switch (event_type)
    {
        case ClientMessage:
            clientMessageEvent(event);
            break;

//        case ConfigureNotify:
//            icon = findIcon(event->xconfigure.window);
//            if (icon)
//                icon->configureEvent(&(event->xconfigure));
//            break;

        case DestroyNotify: {
            unsigned long event_window;
            event_window = reinterpret_cast<xcb_destroy_notify_event_t*>(event)->window;
            icon = findIcon(event_window);
            if (icon)
            {
                icon->windowDestroyed(event_window);
                mIcons.removeAll(icon);
                delete icon;
            }
            break;
        }
        default:
            if (event_type == mDamageEvent + XDamageNotify)
            {
                xcb_damage_notify_event_t* dmg = reinterpret_cast<xcb_damage_notify_event_t*>(event);
                icon = findIcon(dmg->drawable);
                if (icon)
                    icon->updateicon();
            }
            break;
    }
}

QHash<QString, QString> systray::getpluginfo()
{
    QHash<QString, QString> info;
    info["name"] = "System Tray";
    info["needsXcbEvents"] = "true";
    return info;
}

void systray::starttray()
{
    Display* dsp = mDisplay;
    Window root = QX11Info::appRootWindow();
    QString s = QString("_NET_SYSTEM_TRAY_S%1").arg(DefaultScreen(dsp));
    Atom _NET_SYSTEM_TRAY_S = Xcbutills::atom(s.toLatin1());

    if (XGetSelectionOwner(dsp, _NET_SYSTEM_TRAY_S) != None)
    {
        qWarning() << "Another systray is running";
        //mValid = false;
        return;
    }

    // init systray protocol
    mTrayId = XCreateSimpleWindow(dsp, root, -1, -1, 1, 1, 0, 0, 0);

    XSetSelectionOwner(dsp, _NET_SYSTEM_TRAY_S, mTrayId, CurrentTime);
    if (XGetSelectionOwner(dsp, _NET_SYSTEM_TRAY_S) != mTrayId)
    {
        qWarning() << "Can't get systray manager";
        stoptray();
        //mValid = false;
        return;
    }

    int orientation = 0; //0 = horizontal, 1 = vertical
    XChangeProperty(dsp, mTrayId, Xcbutills::atom("_NET_SYSTEM_TRAY_ORIENTATION"), XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&orientation, 1);

    // ** Visual ********************************
    VisualID visualId = getVisual();
    if (visualId)
    {
        XChangeProperty(mDisplay, mTrayId, Xcbutills::atom("_NET_SYSTEM_TRAY_VISUAL"), XA_VISUALID, 32, PropModeReplace, (unsigned char*)&visualId, 1);
    }
    // ******************************************

    setIconSize(iconsize);

    XClientMessageEvent ev;
    ev.type = ClientMessage;
    ev.window = root;
    ev.message_type = Xcbutills::atom("MANAGER");
    ev.format = 32;
    ev.data.l[0] = CurrentTime;
    ev.data.l[1] = long(_NET_SYSTEM_TRAY_S);
    ev.data.l[2] = long(mTrayId);
    ev.data.l[3] = 0;
    ev.data.l[4] = 0;
    XSendEvent(dsp, root, False, StructureNotifyMask, (XEvent*)&ev);

    XDamageQueryExtension(mDisplay, &mDamageEvent, &mDamageError);

    qDebug() << "Systray started";
}

void systray::stoptray()
{
    for (auto & icon : mIcons)
        disconnect(icon, &QObject::destroyed, this, &systray::onIconDestroyed);
    qDeleteAll(mIcons);
    if (mTrayId)
    {
        XDestroyWindow(mDisplay, mTrayId);
        mTrayId = 0;
    }
    //mValid = false;
}

void systray::onIconDestroyed(QObject * icon)
{
    //in the time QOjbect::destroyed is emitted, the child destructor
    //is already finished, so the qobject_cast to child will return nullptr in all cases
    mIcons.removeAll(static_cast<TrayIcon *>(icon));
}

void systray::addIcon(Window winId)
{
    // decline to add an icon for a window we already manage
    TrayIcon *icon = findIcon(winId);
    if(icon)
        return;

    icon = new TrayIcon(winId, iconsize);
    mIcons.append(icon);
    traylayout->addWidget(icon);
    connect(icon, &QObject::destroyed, this, &systray::onIconDestroyed);
}

VisualID systray::getVisual()
{
    VisualID visualId = 0;
    Display* dsp = mDisplay;

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
            if (format && format->type == PictTypeDirect && format->direct.alphaMask)
            {
                visualId = xvi[i].visualid;
                break;
            }
        }
        XFree(xvi);
    }

    return visualId;
}

void systray::setIconSize(QSize icosize)
{
    iconsize = icosize;
    unsigned long size = ulong(qMin(iconsize.width(), iconsize.height()));
    XChangeProperty(mDisplay, mTrayId, Xcbutills::atom("_NET_SYSTEM_TRAY_ICON_SIZE"), XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&size, 1);
}

TrayIcon* systray::findIcon(Window id)
{
    for(TrayIcon* icon : qAsConst(mIcons))
    {
        if (icon->iconId() == id || icon->windowId() == id)
            return icon;
    }
    return nullptr;
}

void systray::clientMessageEvent(xcb_generic_event_t *e)
{
    unsigned long opcode;
    unsigned long message_type;
    Window id;
    xcb_client_message_event_t* event = reinterpret_cast<xcb_client_message_event_t*>(e);
    uint32_t* data32 = event->data.data32;
    message_type = event->type;
    opcode = data32[1];
    if(message_type != _NET_SYSTEM_TRAY_OPCODE)
        return;

    switch (opcode)
    {
        case SYSTEM_TRAY_REQUEST_DOCK:
            id = data32[2];
            if (id)
                addIcon(id);
            break;

        case SYSTEM_TRAY_BEGIN_MESSAGE:
        case SYSTEM_TRAY_CANCEL_MESSAGE:
            qDebug() << "we don't show balloon messages.";
            break;

        default:
//            if (opcode == xfitMan().atom("_NET_SYSTEM_TRAY_MESSAGE_DATA"))
//                qDebug() << "message from dockapp:" << e->data.b;
//            else
//                qDebug() << "SYSTEM_TRAY : unknown message type" << opcode;
            break;
    }
}
