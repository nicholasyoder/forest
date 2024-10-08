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
#include "numlock.h"

#include <QApplication>
#include <QScreen>
#include <KWindowSystem>

#include <netwm.h>

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

QImage Xcbutills::getWindowImage(xcb_window_t window){
    const xcb_get_geometry_cookie_t geoCookie = xcb_get_geometry_unchecked(xcbconnection,  window);
    xcb_get_geometry_reply_t* geo(xcb_get_geometry_reply(xcbconnection, geoCookie, nullptr));
    if (!geo){
        return QImage();
    }

    xcb_image_t *image = xcb_image_get(xcbconnection, window, 0, 0, geo->width, geo->height, 0xFFFFFFFF, XCB_IMAGE_FORMAT_Z_PIXMAP);

    if (image) {
        return QImage(image->data, image->width, image->height, QImage::Format_ARGB32);
    } else {
        QIcon ico = getWindowIcon(window);
        return QImage(ico.pixmap(100,100).toImage());

        //NETWinInfo win_info(xcbconnection, window, QX11Info::appRootWindow(), NET::Properties());

        //QIcon::fromTheme(win_info.iconName());

        //unsigned char *data = win_info.icon().data;
        //return QImage::fromData(data, sizeof(*data));
    }
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

void Xcbutills::enableNumlock(){
    numlock::enableNumlock();
}

QByteArray Xcbutills::get_string_reply(xcb_connection_t *c, const xcb_get_property_cookie_t cookie, xcb_atom_t type){
    xcb_get_property_reply_t *reply = xcb_get_property_reply(c, cookie, nullptr);
    if (!reply)
        return QByteArray();

    QByteArray value;

    if (reply->type == type && reply->format == 8 && reply->value_len > 0){
        const char *data = static_cast<const char *>(xcb_get_property_value(reply));
        int len = int(reply->value_len);
        if (data)
            value = QByteArray(data, data[len - 1] ? len : len - 1);
    }

    free(reply);
    return value;
}

char *Xcbutills::nstrndup(const char *s1, int l){
    if (! s1 || l == 0) {
        return static_cast<char *>(nullptr);
    }

    char *s2 = new char[ulong(l + 1)];
    strncpy(s2, s1, ulong(l));
    s2[l] = '\0';
    return s2;
}

void Xcbutills::send_client_message(xcb_connection_t *c, uint32_t mask, xcb_window_t destination, xcb_window_t window, xcb_atom_t message, const uint32_t data[]){
    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.sequence = 0;
    event.window = window;
    event.type = message;

    for (int i = 0; i < 5; i++) {
        event.data.data32[i] = data[i];
    }

    xcb_send_event(c, false, destination, mask, (const char *) &event);
}

QVector<Xcbutills::xicon> Xcbutills::readxicon(xcb_connection_t *c, const xcb_get_property_cookie_t cookie){
    QVector<xicon> xicons;

    xcb_get_property_reply_t *reply = xcb_get_property_reply(c, cookie, nullptr);
    if (!reply || reply->value_len < 3 || reply->format != 32 || reply->type != XCB_ATOM_CARDINAL) {
        if (reply)
            free(reply);
        return xicons;
    }

    uint32_t *data = (uint32_t *) xcb_get_property_value(reply);
    for (unsigned int i = 0, j = 0; j < reply->value_len - 2; i++) {
        uint32_t width  = data[j++];
        uint32_t height = data[j++];
        uint32_t size   = width * height * sizeof(uint32_t);
        if (j + width * height > reply->value_len) {
            fprintf(stderr, "Ill-encoded icon data; proposed size leads to out of bounds access. Skipping. (%d x %d)\n", width, height);
            break;
        }

        xicon ico(QSize(width, height), new unsigned char[size]);
        memcpy((void *)ico.data, (const void *)&data[j], size);
        xicons.append(ico);

        j += width * height;
    }
    free(reply);
    return xicons;
}
