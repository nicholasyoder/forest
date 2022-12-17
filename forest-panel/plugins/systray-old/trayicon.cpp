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

#include "trayicon.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xdamage.h>

#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>

typedef uchar* UCHARP;

static bool xError;

int windowErrorHandler(Display *d, XErrorEvent *e)
{
    xError = true;
    if (e->error_code != BadWindow)
    {
        char str[1024];
        XGetErrorText(d, e->error_code,  str, 1024);
        qWarning() << "Error handler" << e->error_code << str;
    }
    return 0;
}

//TrayIcon constructor~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TrayIcon::TrayIcon(Window iconId, QWidget* parent): QWidget(parent), m_iconId(iconId), m_windowId(0), m_iconSize(22, 22)
{
    setFixedWidth(26);
    m_iconValid = init();
}

bool TrayIcon::init()
{
    Display* dsp = QX11Info::display();

    XWindowAttributes attr;
    if (! XGetWindowAttributes(dsp, m_iconId, &attr))
        return false;
    unsigned long mask = 0;
    XSetWindowAttributes set_attr;

    Visual* visual = attr.visual;
    set_attr.colormap = attr.colormap;
    set_attr.background_pixel = 0;
    set_attr.border_pixel = 0;
    mask = CWColormap|CWBackPixel|CWBorderPixel;

    m_windowId = XCreateWindow(dsp, this->winId(), 0, 0, uint(m_iconSize.width()), uint(m_iconSize.height()), 0, attr.depth, InputOutput, visual, mask, &set_attr);

    xError = false;
    XErrorHandler old;
    old = XSetErrorHandler(windowErrorHandler);
    XReparentWindow(dsp, m_iconId, m_windowId, 0, 0);
    XSync(dsp, false);
    XSetErrorHandler(old);

    if (xError)
    {
        qWarning() << "* Not icon_swallow *";
        XDestroyWindow(dsp, m_windowId);
        return false;
    }

    Atom acttype;
    int actfmt;
    unsigned long nbitem, bytes;
    unsigned char *data = nullptr;
    int ret;
    ret = XGetWindowProperty(dsp, m_iconId, atom("_XEMBED_INFO"), 0, 2, false, atom("_XEMBED_INFO"), &acttype, &actfmt, &nbitem, &bytes, &data);
    if (ret == Success)
    {
        if (data)
            XFree(data);
    }
    else
    {
        qWarning() << "TrayIcon: xembed error";
        XDestroyWindow(dsp, m_windowId);
        return false;
    }

    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.serial = 0;
    e.xclient.send_event = True;
    e.xclient.message_type = atom("_XEMBED");
    e.xclient.window = m_iconId;
    e.xclient.format = 32;
    e.xclient.data.l[0] = CurrentTime;
    e.xclient.data.l[1] = 0;
    e.xclient.data.l[2] = 0;
    e.xclient.data.l[3] = long(m_windowId);
    e.xclient.data.l[4] = 0;
    XSendEvent(dsp, m_iconId, false, 0xFFFFFF, &e);

    XSelectInput(dsp, m_iconId, StructureNotifyMask);
    m_damage = XDamageCreate(dsp, m_iconId, XDamageReportRawRectangles);
    XCompositeRedirectWindow(dsp, m_windowId, CompositeRedirectManual);
    XMapWindow(dsp, m_iconId);
    XMapRaised(dsp, m_windowId);
    XResizeWindow(QX11Info::display(),m_windowId, uint(m_iconSize.width()), uint(m_iconSize.height()));
    XResizeWindow(QX11Info::display(),m_iconId,   uint(m_iconSize.width()), uint(m_iconSize.height()));

    return true;
}

TrayIcon::~TrayIcon()
{
    Display* dsp = QX11Info::display();
    XSelectInput(dsp, m_iconId, NoEventMask);

    //mDamage
    if (m_damage)
        XDamageDestroy(dsp, m_damage);

    //reparent to root
    xError = false;
    XErrorHandler old = XSetErrorHandler(windowErrorHandler);
    XUnmapWindow(dsp, m_iconId);
    XReparentWindow(dsp, m_iconId, QX11Info::appRootWindow(), 0, 0);
    XDestroyWindow(dsp, m_windowId);
    XSync(dsp, False);
    XSetErrorHandler(old);
}

QSize TrayIcon::sizeHint() const
{
    QMargins margins = contentsMargins();
    return QSize(margins.left() + m_iconSize.width() + margins.right(), margins.top() + m_iconSize.height() + margins.bottom());
}

void TrayIcon::setIconSize(const QSize &iconSize)
{
    m_iconSize = iconSize;

    if (m_windowId)
        XResizeWindow(QX11Info::display(),m_windowId, uint(m_iconSize.width()), uint(m_iconSize.height()));

    if (m_iconId)
        XResizeWindow(QX11Info::display(), m_iconId, uint(m_iconSize.width()), uint(m_iconSize.height()));
}

void TrayIcon::resizeEvent(QResizeEvent *)
{
    QRect rect = iconGeometry();
    XMoveWindow(QX11Info::display(), m_windowId, rect.left(),  rect.top());
}

void TrayIcon::paintEvent(QPaintEvent *event)
{
    draw(event);
}

QRect TrayIcon::iconGeometry()
{
    QRect res = QRect(QPoint(0, 0), m_iconSize);
    res.moveCenter(QRect(0, 0, width(), height()).center());
    return res;
}

void TrayIcon::draw(QPaintEvent* /*event*/)
{
    Display* dsp = QX11Info::display();

    XWindowAttributes attr;
    if (!XGetWindowAttributes(dsp, m_iconId, &attr))
    {
        qWarning() << "Paint error";
        return;
    }

    XImage* ximage = XGetImage(dsp, m_iconId, 0, 0, uint(attr.width), uint(attr.height), AllPlanes, ZPixmap);
    if (!ximage)
    {
        qWarning() << "    * Error image is NULL";
        return;
    }

    QImage image = QImage(UCHARP(ximage->data), ximage->width, ximage->height, ximage->bytes_per_line,  QImage::Format_ARGB32_Premultiplied);

    //Draw QImage ...........................
    QPainter painter(this);

    QRect iconRect = iconGeometry();

    if (image.size() != iconRect.size())
    {
              image = image.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
              QRect r = image.rect();
              r.moveCenter(iconRect.center());
              iconRect = r;
    }
    painter.drawImage(iconRect, image);

    XDestroyImage(ximage);
}

Atom TrayIcon::atom(const char* atomName)
{
    static QHash<QString, Atom> hash;

    if (hash.contains(atomName))
        return hash.value(atomName);

    Atom atom = XInternAtom(QX11Info::display(), atomName, false);
    hash[atomName] = atom;
    return atom;
}
