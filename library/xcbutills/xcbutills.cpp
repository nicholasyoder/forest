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

#include "xcbutills.h"
#include <KWindowSystem>

#include "xcb/xcb_image.h"

xcb_connection_t* Xcbutills::xcbconnection = QX11Info::connection();

xcb_atom_t Xcbutills::atom(QString name){
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(xcbconnection, 1, uint16_t(name.toLatin1().length()), name.toLatin1().data());
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(xcbconnection, cookie, nullptr);
    return reply->atom;
}

char* Xcbutills::getAtomName(xcb_atom_t atom){
    xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(xcbconnection, atom);
    return xcb_get_atom_name_name(xcb_get_atom_name_reply(xcbconnection, cookie, nullptr));
}

QList<xcb_window_t> Xcbutills::getClientList(){
    xcb_get_property_cookie_t cookie = xcb_get_property(xcbconnection, false, xcb_window_t(QX11Info::appRootWindow()), atom("_NET_CLIENT_LIST"), XCB_ATOM_WINDOW, 0, 100000);
    QVector<xcb_window_t> clients = get_array_reply<xcb_window_t>(xcbconnection, cookie, XCB_ATOM_WINDOW);
    QList<xcb_window_t> clientlist = clients.toList();
    foreach(xcb_window_t window, clientlist)
    {
        if (!isWindow4Taskbar(window))
            clientlist.removeOne(window);
    }
    return clientlist;
}

bool Xcbutills::isWindow4Taskbar(xcb_window_t window){
    xcb_get_property_cookie_t cookie = xcb_get_property(xcbconnection, false, window, atom("_NET_WM_WINDOW_TYPE"), XCB_ATOM_ATOM, 0, 2048);
    const QVector<xcb_atom_t> types = get_array_reply<xcb_atom_t>(xcbconnection, cookie, XCB_ATOM_ATOM);

    if (types.contains(atom("_NET_WM_WINDOW_TYPE_DESKTOP")) || types.contains(atom("_NET_WM_WINDOW_TYPE_DOCK")) ||
            types.contains(atom("_NET_WM_WINDOW_TYPE_SPLASH")) || types.contains(atom("_NET_WM_WINDOW_TYPE_TOOLBAR")) ||
            types.contains(atom("_NET_WM_WINDOW_TYPE_MENU")) || types.contains(atom("_NET_WM_WINDOW_TYPE_POPUP_MENU")))
        return false;
    else
        return true;
}

QString Xcbutills::getWindowTitle(xcb_window_t window){
    //TODO: try _NET_WM_VISIBLE_NAME, _NET_WM_NAME, WM_NAME before returning empty
    /*xcb_get_property_cookie_t cookie = xcb_get_property(xcbconnection, false, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0, 100000);

    QString s = "unknown";
    const QByteArray str = get_string_reply(xcbconnection, cookie, XCB_ATOM_STRING);
    if (str.length() > 0)
        s = nstrndup(str.constData(), str.length());
    //else
    //qDebug() << "error - can't get WM_NAME";

    return s;*/
    KWindowInfo info(window, NET::WMVisibleName | NET::WMName);
    QString title = info.visibleName().isEmpty() ? info.name() : info.visibleName();
    return title;
}

QIcon Xcbutills::getWindowIcon(xcb_window_t window)
{
    xcb_get_property_cookie_t cookie = xcb_get_property(xcbconnection, false, window, atom("_NET_WM_ICON"), XCB_ATOM_CARDINAL, 0, 0xffffffff);

    QVector<xicon> icons = readxicon(xcbconnection, cookie);

    QIcon ico;
    for (int c = 0; c < icons.size(); c++)
    {
        uchar *data = icons[c].data;
        int width = icons[c].size.width();
        int height = icons[c].size.height();

        if (data && width > 0 && height > 0){
            QImage img(data, width, height, QImage::Format_ARGB32);
            if (!img.isNull())
                ico.addPixmap(QPixmap::fromImage(img));
        }

        delete data;
    }

    if (ico.isNull())
        return QIcon::fromTheme("unknown");

    return ico;
}

int Xcbutills::getWindowDesktop(xcb_window_t window)
{
    xcb_get_property_cookie_t cookie = xcb_get_property(xcbconnection, false, window, atom("_NET_WM_DESKTOP"), XCB_ATOM_CARDINAL, 0, 100000);
    bool success;
    uint32_t desktop = get_value_reply<uint32_t>(xcbconnection, cookie, XCB_ATOM_CARDINAL, 0, &success);
    if (success)
    {
        if (desktop != 0xffffffff)
            return int(desktop) + 1;
        else
            return -1;//On all desktops
    }
    else {
        return 0;//error
    }
}

QImage Xcbutills::getWindowImage(xcb_window_t window)
{
    const xcb_get_geometry_cookie_t geoCookie = xcb_get_geometry_unchecked(xcbconnection,  window);
    xcb_get_geometry_reply_t* geo(xcb_get_geometry_reply(xcbconnection, geoCookie, nullptr));
    if (!geo){
        return QImage();
    }


    //xcb_connection_t *con = xcb_connect(nullptr, nullptr);

    //xcb_map_window(xcbconnection, window);
    //xcb_clear_area(xcbconnection, 1, window, 0, 0, geo->width, geo->height);
    //xcb_flush(xcbconnection);


    xcb_image_t *image = xcb_image_get(xcbconnection, window, 0, 0, geo->width, geo->height, 0xFFFFFFFF, XCB_IMAGE_FORMAT_Z_PIXMAP);

    if (image) {
        return QImage(image->data, image->width, image->height, QImage::Format_ARGB32);
    } else {
        return QImage();
    }

    /*const xcb_get_image_cookie_t imageCookie = xcb_get_image_unchecked(con, XCB_IMAGE_FORMAT_Z_PIXMAP, window, 0, 0, geo->width, geo->height, uint32_t(~0));
    xcb_get_image_reply_t* xImage(xcb_get_image_reply(con, imageCookie, nullptr));
    if (!xImage) { // for some unknown reason, xcb_get_image failed. try another less efficient method.
        qDebug() << "Xcbutils: falling back to screen->grabWindow";
        xcb_clear_area(xcbconnection, 1, window, 0, 0, geo->width, geo->height);
        return qApp->primaryScreen()->grabWindow(window).toImage();
    }

    QImage::Format format = QImage::Format_Invalid;
    switch (xImage->depth) {
    case 1: format = QImage::Format_MonoLSB; break;
    case 16: format = QImage::Format_RGB16; break;
    case 24: format = QImage::Format_RGB32; break;
    case 30: {
        // Qt doesn't have a matching image format. We need to convert manually
        uint32_t *pixels = reinterpret_cast<uint32_t *>(xcb_get_image_data(xImage));
        for (uint i = 0; i < xImage->length; ++i)
        {
            int r = (pixels[i] >> 22) & 0xff;
            int g = (pixels[i] >> 12) & 0xff;
            int b = (pixels[i] >>  2) & 0xff;

            pixels[i] = qRgba(r, g, b, 0xff);
        }
        // fall through, Qt format is still Format_ARGB32_Premultiplied
        Q_FALLTHROUGH();
    }
    case 32: format = QImage::Format_ARGB32_Premultiplied; break;
    default: return QImage(); // we don't know
    }

    QImage image(xcb_get_image_data(xImage), geo->width, geo->height, xcb_get_image_data_length(xImage) / geo->height, format, free, xImage);

    if (image.isNull()) {return QImage();}

    if (image.format() == QImage::Format_MonoLSB) {
        // work around an abort in QImage::color
        image.setColorCount(2);
        image.setColor(0, QColor(Qt::white).rgb());
        image.setColor(1, QColor(Qt::black).rgb());
    }
    return image;*/
}

int Xcbutills::getNumDesktops()
{
    xcb_get_property_cookie_t cookie = xcb_get_property(xcbconnection, false, xcb_window_t(QX11Info::appRootWindow()), atom("_NET_NUMBER_OF_DESKTOPS"), XCB_ATOM_CARDINAL, 0, 1);
    return int(get_value_reply<uint32_t>(xcbconnection, cookie, XCB_ATOM_CARDINAL, 0));
}

//get the active desktop
int Xcbutills::getCurrentDesktop()
{
    xcb_get_property_cookie_t cookie = xcb_get_property(xcbconnection, false, xcb_window_t(QX11Info::appRootWindow()), atom("_NET_CURRENT_DESKTOP"), XCB_ATOM_CARDINAL, 0, 1);
    return int(get_value_reply<uint32_t>(xcbconnection, cookie, XCB_ATOM_CARDINAL, 0) + 1);
}

//show/unshow desktop
void Xcbutills::showDesktop()
{
    xcb_get_property_cookie_t cookie = xcb_get_property(xcbconnection, false, xcb_window_t(QX11Info::appRootWindow()), atom("_NET_SHOWING_DESKTOP"), XCB_ATOM_CARDINAL, 0, 1);
    bool shown = bool(get_value_reply<uint32_t>(xcbconnection, cookie, XCB_ATOM_CARDINAL, 0));

    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.sequence = 0;
    event.window = xcb_window_t(QX11Info::appRootWindow());
    event.type = atom("_NET_SHOWING_DESKTOP");
    event.data.data32[0] = !shown;

    uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    xcb_send_event(xcbconnection, false, xcb_window_t(QX11Info::appRootWindow()), sendevent_mask, (const char *) &event);
}

//get active window
xcb_window_t Xcbutills::getActiveWindow()
{
    xcb_get_property_cookie_t cookie = xcb_get_property(xcbconnection, false, xcb_window_t(QX11Info::appRootWindow()), atom("_NET_ACTIVE_WINDOW"), XCB_ATOM_WINDOW, 0, 1);
    return get_value_reply<xcb_window_t>(xcbconnection, cookie, XCB_ATOM_WINDOW, 0);
}

//activate window
void Xcbutills::raiseWindow(xcb_window_t window)
{
    uint source = 0;//0 = unknown, 1 = normal application, 2 = pager or similer
    xcb_timestamp_t timestamp = uint(QX11Info::appUserTime());
    xcb_window_t active_window = XCB_WINDOW_NONE;
    const uint32_t data[5] = {source, timestamp, active_window, 0, 0};
    uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    send_client_message(xcbconnection, sendevent_mask, xcb_window_t(QX11Info::appRootWindow()), window, atom("_NET_ACTIVE_WINDOW"), data);
}

void Xcbutills::maximizeWindow(xcb_window_t window)
{
    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.sequence = 0;
    event.window = window;
    event.type = atom("_NET_WM_STATE");
    event.data.data32[0] = 1;
    event.data.data32[1] = atom("_NET_WM_STATE_MAXIMIZED_HORZ");
    event.data.data32[2] = atom("_NET_WM_STATE_MAXIMIZED_VERT");
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;
    uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    xcb_send_event(xcbconnection, false, xcb_window_t(QX11Info::appRootWindow()), sendevent_mask, (const char *) &event);
}

void Xcbutills::demaximizeWindow(xcb_window_t window)
{
    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.sequence = 0;
    event.window = window;
    event.type = atom("_NET_WM_STATE");
    event.data.data32[0] = 0;
    event.data.data32[1] = atom("_NET_WM_STATE_MAXIMIZED_HORZ");
    event.data.data32[2] = atom("_NET_WM_STATE_MAXIMIZED_VERT");
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;
    uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    xcb_send_event(xcbconnection, false, xcb_window_t(QX11Info::appRootWindow()), sendevent_mask, (const char *) &event);
}

void Xcbutills::minimizeWindow(xcb_window_t window)
{
    xcb_client_message_event_t ev;
    memset(&ev, 0, sizeof(ev));
    ev.response_type = XCB_CLIENT_MESSAGE;
    ev.window = window;
    ev.type = atom("WM_CHANGE_STATE");
    ev.format = 32;
    ev.data.data32[0] = 3;//_ICCCM_WM_STATE_ICONIC
    ev.data.data32[1] = 0;
    ev.data.data32[2] = 0;
    ev.data.data32[3] = 0;
    ev.data.data32[4] = 0;
    uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    xcb_send_event(xcbconnection, false, xcb_window_t(QX11Info::appRootWindow()), sendevent_mask, reinterpret_cast<const char*>(&ev));
}

void Xcbutills::closeWindow(xcb_window_t window)
{
    const uint32_t data[5] = { 0, 0, 0, 0, 0 };
    uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    send_client_message(xcbconnection, sendevent_mask, xcb_window_t(QX11Info::appRootWindow()), window, atom("_NET_CLOSE_WINDOW"), data);
}

void Xcbutills::resizeWindow(xcb_window_t window, int w, int h)
{
    const uint32_t data[5] = { uint32_t(w), uint32_t(h), uint32_t(6), 0, 0 };
    uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    send_client_message(xcbconnection, sendevent_mask, xcb_window_t(QX11Info::appRootWindow()), window, atom("_NET_WM_MOVERESIZE"), data);
}

void Xcbutills::moveWindow(xcb_window_t window, int x, int y)
{
    uint32_t configVals[2] = {0, 0};
    configVals[0] = static_cast<uint32_t>(x);
    configVals[1] = static_cast<uint32_t>(y);
    xcb_configure_window(xcbconnection, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, configVals);
}

void Xcbutills::fitWindowOnScreen(xcb_window_t window){
    demaximizeWindow(window);
    KWindowInfo info(window, NET::WMGeometry);
    QRect screengeo = qApp->primaryScreen()->geometry();
    QRect windowgeo = info.geometry();
    int x = (windowgeo.width() > screengeo.width()) ? 50 : screengeo.width()/2 - windowgeo.width()/2;
    int y = (windowgeo.height() > screengeo.height()) ? 50 : screengeo.height()/2 - windowgeo.height()/2;
    moveWindow(window, x, y);
}

void Xcbutills::setCurrentDesktop(int desknum)
{
    const uint32_t data[5] = {uint32_t(desknum - 1), 0, 0, 0, 0};
    uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    send_client_message(xcbconnection, sendevent_mask, xcb_window_t(QX11Info::appRootWindow()),
                        xcb_window_t(QX11Info::appRootWindow()), atom("_NET_CURRENT_DESKTOP"), data);
}

//not sure what this actually does -- setPartialStrut seems to be what should be used
/*void Xcbutills::setStrut(xcb_window_t window, QRect strut)
{
    uint32_t data[4];
    data[0] = strut.left();
    data[1] = strut.right();
    data[2] = strut.top();
    data[3] = strut.bottom();

    xcb_change_property(xcbconnection, XCB_PROP_MODE_REPLACE, window, atom("_NET_WM_STRUT"), XCB_ATOM_CARDINAL, 32, 4, (const void *) data);
}*/

void Xcbutills::setPartialStrut(xcb_window_t window, int left_width, int right_width, int top_width, int bottom_width,
                            int left_start, int left_end, int right_start, int right_end, int top_start, int top_end, int bottom_start, int bottom_end)
{
    uint32_t data[12];
    data[0] = left_width;
    data[1] = right_width;
    data[2] = top_width;
    data[3] = bottom_width;
    data[4] = left_start;
    data[5] = left_end;
    data[6] = right_start;
    data[7] = right_end;
    data[8] = top_start;
    data[9] = top_end;
    data[10] = bottom_start;
    data[11] = bottom_end;

    xcb_change_property(xcbconnection, XCB_PROP_MODE_REPLACE, window, atom("_NET_WM_STRUT_PARTIAL"), XCB_ATOM_CARDINAL, 32, 12, (const void *) data);
}

