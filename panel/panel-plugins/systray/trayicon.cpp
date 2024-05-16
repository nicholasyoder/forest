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

// Warning: order of include is important.
#include <QDebug>
#include <QApplication>
#include <QResizeEvent>
#include <QPainter>
#include <QBitmap>
#include <QStyle>
#include <QScreen>
#include <QLabel>

#include "trayicon.h"
#include "xcbutills/xcbutills.h"

#include <QtX11Extras/QX11Info>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xrender.h>

#include <xcb/composite.h>

#define XEMBED_EMBEDDED_NOTIFY 0
static bool xError;

int windowErrorHandler(Display *d, XErrorEvent *e)
{
    xError = true;
    if (e->error_code != BadWindow) {
        char str[1024];
        XGetErrorText(d, e->error_code,  str, 1024);
        qWarning() << "SystemTray: " << e->error_code << str;
    }
    return 0;
}

TrayIcon::TrayIcon(Window iconId, QSize const & iconSize) : mIconId(iconId), mWindowId(0), mIconSize(iconSize), mDamage(0), mDisplay(QX11Info::display())
{
    // NOTE:
    // it's a good idea to save the return value of QX11Info::display().
    // In Qt 5, this API is slower and has some limitations which can trigger crashes.
    // The XDisplay value is actally stored in QScreen object of the primary screen rather than
    // in a global variable. So when the parimary QScreen is being deleted and becomes invalid,
    // QX11Info::display() will fail and cause crash. Storing this value improves the efficiency and
    // also prevent potential crashes caused by this bug.

    ///setObjectName("TrayIcon");
    ///setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // NOTE:
    // see https://github.com/lxqt/lxqt/issues/945
    // workaround: delayed init because of weird behaviour of some icons/windows (claws-mail)
    // (upon starting the app the window for receiving clicks wasn't correctly sized
    //  no matter what we've done)

    //connect(t, &QTimer::timeout, this, &TrayIcon::tryleave);

    QTimer::singleShot(200, this, &TrayIcon::init);
}

void TrayIcon::init()
{
    Display* dsp = mDisplay;

    XWindowAttributes attr;
    if (! XGetWindowAttributes(dsp, mIconId, &attr)){
        deleteLater();
        return;
    }

    unsigned long mask = 0;
    XSetWindowAttributes set_attr;

    Visual* visual = attr.visual;
    set_attr.colormap = attr.colormap;
    set_attr.background_pixel = 0;
    set_attr.border_pixel = 0;
    mask = CWColormap|CWBackPixel|CWBorderPixel;

    const QRect icon_geom = iconGeometry();
    mWindowId = XCreateWindow(dsp, this->winId(), icon_geom.x(), icon_geom.y(), uint(icon_geom.width() * metric(PdmDevicePixelRatio)),
                              uint(icon_geom.height() * metric(PdmDevicePixelRatio)), 0, attr.depth, InputOutput, visual, mask, &set_attr);

    xError = false;
    XErrorHandler old;
    old = XSetErrorHandler(windowErrorHandler);
    XReparentWindow(dsp, mIconId, mWindowId, 0, 0);
    XSync(dsp, false);
    XSetErrorHandler(old);

    if (xError){
        qWarning() << "SystemTray: Not icon_swallow";
        XDestroyWindow(dsp, mWindowId);
        mWindowId = 0;
        deleteLater();
        return;
    }


    {
        Atom acttype;
        int actfmt;
        unsigned long nbitem, bytes;
        unsigned char *data = nullptr;
        int ret;

        ret = XGetWindowProperty(dsp, mIconId, Xcbutills::atom("_XEMBED_INFO"),
                                 0, 2, false, Xcbutills::atom("_XEMBED_INFO"),
                                 &acttype, &actfmt, &nbitem, &bytes, &data);
        if (ret == Success)
        {
            if (data)
                XFree(data);
        }
        else
        {
            qWarning() << "SystemTray: xembed error";
            XDestroyWindow(dsp, mWindowId);
            deleteLater();
            return;
        }
    }

    {
        /*XEvent e;
        e.xclient.type = ClientMessage;
        e.xclient.serial = 0;
        e.xclient.send_event = True;
        e.xclient.message_type = Xcbutills::atom("_XEMBED");
        e.xclient.window = mIconId;
        e.xclient.format = 32;
        e.xclient.data.l[0] = CurrentTime;
        e.xclient.data.l[1] = XEMBED_EMBEDDED_NOTIFY;
        e.xclient.data.l[2] = 0;
        e.xclient.data.l[3] = long(mWindowId);
        e.xclient.data.l[4] = 0;
        XSendEvent(dsp, mIconId, false, 0xFFFFFF, &e);
        */

        xcb_client_message_event_t event;
        event.response_type = XCB_CLIENT_MESSAGE;
        event.format = 32;
        event.sequence = 0;
        event.window = mIconId;
        event.type = Xcbutills::atom("_XEMBED");
        event.data.data32[0] = CurrentTime;
        event.data.data32[1] = XEMBED_EMBEDDED_NOTIFY;
        event.data.data32[2] = 0;
        event.data.data32[3] = long(mWindowId);
        event.data.data32[4] = 0;
        uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
        xcb_send_event(QX11Info::connection(), false, mIconId, sendevent_mask, (const char *) &event);
    }

    XSelectInput(dsp, mIconId, StructureNotifyMask);
    mDamage = XDamageCreate(dsp, mIconId, XDamageReportRawRectangles);
    //XCompositeRedirectWindow(dsp, mWindowId, CompositeRedirectManual);
    xcb_composite_redirect_window(QX11Info::connection(), xcb_window_t(mWindowId), CompositeRedirectManual);

    //XMapWindow(dsp, mIconId);
    xcb_map_window(QX11Info::connection(), xcb_window_t(mIconId));
    //XMapRaised(dsp, mWindowId);
    xcb_map_window_checked(QX11Info::connection(), xcb_window_t(mWindowId));

    const QSize req_size{mIconSize * metric(PdmDevicePixelRatio)};
    //XResizeWindow(dsp, mIconId, uint(req_size.width()), uint(req_size.height()));
    XResizeWindow(dsp, mIconId, uint(req_size.width()), uint(req_size.height()));

    this->updateicon();
}

TrayIcon::~TrayIcon()
{
    Display* dsp = mDisplay;
    XSelectInput(dsp, mIconId, NoEventMask);

    if (mDamage)
        XDamageDestroy(dsp, mDamage);

    // reparent to root
    xError = false;
    XErrorHandler old = XSetErrorHandler(windowErrorHandler);

    XUnmapWindow(dsp, mIconId);
    XReparentWindow(dsp, mIconId, QX11Info::appRootWindow(), 0, 0);

    if (mWindowId)
        XDestroyWindow(dsp, mWindowId);
    XSync(dsp, False);
    XSetErrorHandler(old);
}

QSize TrayIcon::sizeHint() const
{
    QMargins margins = contentsMargins();
    return QSize(margins.left() + mIconSize.width() + margins.right(), margins.top() + mIconSize.height() + margins.bottom());
}

void TrayIcon::setIconSize(QSize iconSize)
{
    mIconSize = iconSize;

    const QSize req_size{mIconSize * metric(PdmDevicePixelRatio)};
    if (mWindowId)
        Xcbutills::resizeWindow(xcb_window_t(mWindowId), req_size.width(), req_size.height());

    if (mIconId)
        Xcbutills::resizeWindow(xcb_window_t(mIconId), req_size.width(), req_size.height());
}

QRect TrayIcon::iconGeometry()
{
    QRect res = QRect(QPoint(0, 0), mIconSize);

    res.moveCenter(QRect(0, 0, width(), height()).center());
    return res;
}

void TrayIcon::updateicon()
{
    update();
}

void TrayIcon::paintEvent(QPaintEvent* event)
{
    //panelbutton::paintEvent(event);

    Display* dsp = mDisplay;
    XWindowAttributes attr;
    if (!XGetWindowAttributes(dsp, mIconId, &attr))
    {
        qWarning() << "systray: Paint error";
        return;
    }

    XImage* ximage = XGetImage(dsp, mIconId, 0, 0, uint(attr.width), uint(attr.height), AllPlanes, ZPixmap);
    if(ximage)
    {
        iconimage = QImage((const uchar*) ximage->data, ximage->width, ximage->height, ximage->bytes_per_line,  QImage::Format_ARGB32_Premultiplied);
    }
    else {
        iconimage = qApp->primaryScreen()->grabWindow(mIconId, 0,0, attr.width, attr.height).toImage();
    }


    QPainter painter(this);
    QRect iconRect = iconGeometry();
    if (iconimage.size() != iconRect.size())
    {
        iconimage = iconimage.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect r = iconimage.rect();
        r.moveCenter(iconRect.center());
        iconRect = r;
    }
    painter.drawImage(iconRect, iconimage);
}

void TrayIcon::windowDestroyed(Window w)
{
    //damage is destroyed if it's parent window was destroyed
    if (mIconId == w)
        mDamage = 0;
}

bool TrayIcon::isXCompositeAvailable()
{
    int eventBase, errorBase;
    return XCompositeQueryExtension(QX11Info::display(), &eventBase, &errorBase );
}
