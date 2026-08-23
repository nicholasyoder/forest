// SPDX-License-Identifier: LGPL-3.0-or-later

#include "xcbutills.h"
//#include "numlock.h"

#include <QApplication>
#include <QScreen>
#include <KWindowSystem>
#include <KWindowInfo>

#include <netwm.h>
#include "xcb/xcb_image.h"

static xcb_connection_t* init_x11_connection(){
    auto *x11App = qApp->nativeInterface<QNativeInterface::QX11Application>();
    return x11App ? x11App->connection() : nullptr;
}

xcb_connection_t* Xcbutills::conn = init_x11_connection();

Display* Xcbutills::display(){
    auto *x11App = qApp->nativeInterface<QNativeInterface::QX11Application>();
    return x11App ? x11App->display() : nullptr;
}

xcb_atom_t Xcbutills::atom(QString name){
    QByteArray name_bytes = name.toLatin1();
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, 0, uint16_t(name_bytes.length()), name_bytes.data());
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(conn, cookie, nullptr);
    xcb_atom_t atom = reply->atom;
    free(reply);
    return atom;
}

char* Xcbutills::getAtomName(xcb_atom_t atom){
    xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(conn, atom);
    return xcb_get_atom_name_name(xcb_get_atom_name_reply(conn, cookie, nullptr));
}

xcb_window_t Xcbutills::root_window(){
    return xcb_setup_roots_iterator(xcb_get_setup(conn)).data->root;
}

QList<xcb_window_t> Xcbutills::getClientList(){
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, xcb_window_t(root_window()), atom("_NET_CLIENT_LIST"), XCB_ATOM_WINDOW, 0, 100000);
    QVector<xcb_window_t> clients = get_array_reply<xcb_window_t>(conn, cookie, XCB_ATOM_WINDOW);
    QList<xcb_window_t> clientlist = clients.toList();
    foreach(xcb_window_t window, clientlist){
        if (!isWindow4Taskbar(window))
            clientlist.removeOne(window);
    }
    return clientlist;
}

bool Xcbutills::isWindow4Taskbar(xcb_window_t window){
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, window, atom("_NET_WM_WINDOW_TYPE"), XCB_ATOM_ATOM, 0, 2048);
    const QVector<xcb_atom_t> types = get_array_reply<xcb_atom_t>(conn, cookie, XCB_ATOM_ATOM);

    if (types.contains(atom("_NET_WM_WINDOW_TYPE_DESKTOP")) || types.contains(atom("_NET_WM_WINDOW_TYPE_DOCK")) ||
            types.contains(atom("_NET_WM_WINDOW_TYPE_SPLASH")) || types.contains(atom("_NET_WM_WINDOW_TYPE_TOOLBAR")) ||
            types.contains(atom("_NET_WM_WINDOW_TYPE_MENU")) || types.contains(atom("_NET_WM_WINDOW_TYPE_POPUP_MENU")))
        return false;
    else
        return true;
}

QString Xcbutills::getWindowTitle(xcb_window_t window){
    //TODO: try _NET_WM_VISIBLE_NAME, _NET_WM_NAME, WM_NAME before returning empty
    /*xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0, 100000);

    QString s = "unknown";
    const QByteArray str = get_string_reply(conn, cookie, XCB_ATOM_STRING);
    if (str.length() > 0)
        s = nstrndup(str.constData(), str.length());
    //else
    //qDebug() << "error - can't get WM_NAME";

    return s;*/
    KWindowInfo info(window, NET::WMVisibleName | NET::WMName);
    QString title = info.visibleName().isEmpty() ? info.name() : info.visibleName();
    return title;
}

QIcon Xcbutills::getWindowIcon(xcb_window_t window){
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, window, atom("_NET_WM_ICON"), XCB_ATOM_CARDINAL, 0, 0xffffffff);
    QVector<xicon> icons = readxicon(conn, cookie);
    QIcon ico;
    for (int c = 0; c < icons.size(); c++){
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
    if (ico.isNull()) return QIcon::fromTheme("unknown");

    return ico;
}

int Xcbutills::getWindowDesktop(xcb_window_t window){
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, window, atom("_NET_WM_DESKTOP"), XCB_ATOM_CARDINAL, 0, 100000);
    bool success;
    uint32_t desktop = get_value_reply<uint32_t>(conn, cookie, XCB_ATOM_CARDINAL, 0, &success);
    if (success){
        if (desktop != 0xffffffff)
            return int(desktop) + 1;
        else
            return -1;//On all desktops
    }
    else {
        return 0;//error
    }
}

// Check if all pixels in an image are transparent
bool is_all_transparent(QImage image){
    for(int x = 0; x < image.width(); x++){
        for(int y = 0; y < image.height(); y++){
            if(image.pixelColor(x,y).alpha() != 0)
                return false;
        }
    }
    return true;
}

// Get a screenshot of the specified window and scale it to the target height.
// Falls back to the window icon when unable to retrieve a screenshot (minimized windows).
QPixmap Xcbutills::getWindowImage(xcb_window_t window, int target_height){
    const xcb_get_geometry_cookie_t geoCookie = xcb_get_geometry_unchecked(conn,  window);
    xcb_get_geometry_reply_t* geo(xcb_get_geometry_reply(conn, geoCookie, nullptr));
    if (!geo) return QPixmap();

    xcb_image_t *image = xcb_image_get(conn, window, 0, 0, geo->width, geo->height, 0xFFFFFFFF, XCB_IMAGE_FORMAT_Z_PIXMAP);
    if (image) {
        QImage q_image = QImage(image->data, image->width, image->height, QImage::Format_ARGB32);
        if(!q_image.isNull()){
            // Get rid of alpha channel if entire image is transparent. Happens for some hidden windows.
            if(q_image.pixelColor(0,0).alpha() == 0 && is_all_transparent(q_image))
                q_image = q_image.convertToFormat(QImage::Format_RGB32);

            QPixmap pix = QPixmap::fromImage(q_image);
            if (pix.height() > target_height) // Scale image if needed.
                pix = pix.scaledToHeight(target_height, Qt::SmoothTransformation);
            return pix;
        }
    }

    // Fall back to window icon
    QIcon ico = getWindowIcon(window);
    return ico.pixmap(target_height, target_height);
}

int Xcbutills::getNumDesktops(){
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, xcb_window_t(root_window()), atom("_NET_NUMBER_OF_DESKTOPS"), XCB_ATOM_CARDINAL, 0, 1);
    return int(get_value_reply<uint32_t>(conn, cookie, XCB_ATOM_CARDINAL, 0));
}

//get the active desktop
int Xcbutills::getCurrentDesktop(){
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, xcb_window_t(root_window()), atom("_NET_CURRENT_DESKTOP"), XCB_ATOM_CARDINAL, 0, 1);
    return int(get_value_reply<uint32_t>(conn, cookie, XCB_ATOM_CARDINAL, 0) + 1);
}

//show/unshow desktop
void Xcbutills::showDesktop(){
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, xcb_window_t(root_window()), atom("_NET_SHOWING_DESKTOP"), XCB_ATOM_CARDINAL, 0, 1);
    bool shown = bool(get_value_reply<uint32_t>(conn, cookie, XCB_ATOM_CARDINAL, 0));

    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.sequence = 0;
    event.window = xcb_window_t(root_window());
    event.type = atom("_NET_SHOWING_DESKTOP");
    event.data.data32[0] = !shown;

    uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    xcb_send_event(conn, false, xcb_window_t(root_window()), sendevent_mask, (const char *) &event);
}

//get active window
xcb_window_t Xcbutills::getActiveWindow(){
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, xcb_window_t(root_window()), atom("_NET_ACTIVE_WINDOW"), XCB_ATOM_WINDOW, 0, 1);
    return get_value_reply<xcb_window_t>(conn, cookie, XCB_ATOM_WINDOW, 0);
}

//activate window
void Xcbutills::raiseWindow(xcb_window_t window){
    uint source = 0;//0 = unknown, 1 = normal application, 2 = pager or similer
    const uint32_t data[5] = {source, XCB_CURRENT_TIME, XCB_WINDOW_NONE, 0, 0};
    uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    send_client_message(conn, sendevent_mask, xcb_window_t(root_window()), window, atom("_NET_ACTIVE_WINDOW"), data);
}

void Xcbutills::maximizeWindow(xcb_window_t window){
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
    xcb_send_event(conn, false, xcb_window_t(root_window()), sendevent_mask, (const char *) &event);
}

void Xcbutills::demaximizeWindow(xcb_window_t window){
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
    xcb_send_event(conn, false, xcb_window_t(root_window()), sendevent_mask, (const char *) &event);
}

void Xcbutills::minimizeWindow(xcb_window_t window){
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
    xcb_send_event(conn, false, xcb_window_t(root_window()), sendevent_mask, reinterpret_cast<const char*>(&ev));
}

void Xcbutills::closeWindow(xcb_window_t window){
    const uint32_t data[5] = { 0, 0, 0, 0, 0 };
    uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    send_client_message(conn, sendevent_mask, xcb_window_t(root_window()), window, atom("_NET_CLOSE_WINDOW"), data);
}

void Xcbutills::resizeWindow(xcb_window_t window, int w, int h){
    const uint32_t data[5] = { uint32_t(w), uint32_t(h), uint32_t(6), 0, 0 };
    uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    send_client_message(conn, sendevent_mask, xcb_window_t(root_window()), window, atom("_NET_WM_MOVERESIZE"), data);
}

void Xcbutills::moveWindow(xcb_window_t window, int x, int y){
    uint32_t configVals[2] = {0, 0};
    configVals[0] = static_cast<uint32_t>(x);
    configVals[1] = static_cast<uint32_t>(y);
    xcb_configure_window(conn, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, configVals);
}

void Xcbutills::moveWindowToDesktop(xcb_window_t window, int desktop){
    uint32_t desktop_value = static_cast<uint32_t>(desktop - 1);
    uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    send_client_message(conn, sendevent_mask, xcb_window_t(root_window()), window, atom("_NET_WM_DESKTOP"), &desktop_value);
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

void Xcbutills::setCurrentDesktop(int desknum){
    const uint32_t data[5] = {uint32_t(desknum - 1), 0, 0, 0, 0};
    uint sendevent_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    xcb_window_t root_w = root_window();
    send_client_message(conn, sendevent_mask, root_w, root_w, atom("_NET_CURRENT_DESKTOP"), data);
}

//void Xcbutills::enableNumlock(){
//    numlock::enableNumlock();
//}

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
