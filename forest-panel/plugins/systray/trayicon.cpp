/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2022 Nicholas Yoder
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

#include <xcb/composite.h>
#include <xcb/xcb_image.h>

#include "xcbutills.h"

#include <QX11Info>
#include <QTimer>
#include <QBitmap>

#include <netwm.h>

#include <math.h>

static uint16_t s_embedSize = 32; //max size of window to embed. We no longer resize the embedded window as Chromium acts stupidly.
static unsigned int XEMBED_VERSION = 0;

void xembed_message_send(xcb_window_t towin, long message, long d1, long d2, long d3){
    xcb_client_message_event_t ev;
    ev.response_type = XCB_CLIENT_MESSAGE;
    ev.window = towin;
    ev.format = 32;
    ev.data.data32[0] = XCB_CURRENT_TIME;
    ev.data.data32[1] = message;
    ev.data.data32[2] = d1;
    ev.data.data32[3] = d2;
    ev.data.data32[4] = d3;
    ev.type = Xcbutills::atom("_XEMBED");
    xcb_send_event(QX11Info::connection(), false, towin, XCB_EVENT_MASK_NO_EVENT, (char *) &ev);
}

trayicon::trayicon(xcb_window_t wid): iWindowId(wid), sendingClickEvent(false), iInjectMode(Direct){
    setupIconButton(QIcon::fromTheme("unknown"));
    connect(this, &trayicon::mouseReleased, this, &trayicon::handleClick);
    init();
}

trayicon::~trayicon(){
    auto c = QX11Info::connection();
    xcb_destroy_window(c, iContainerWid);
}

void trayicon::init(){
    auto c = QX11Info::connection();

    //create a container window
    auto screen = xcb_setup_roots_iterator (xcb_get_setup (c)).data;
    iContainerWid = xcb_generate_id(c);
    uint32_t values[3];
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
    values[0] = screen->black_pixel; //draw a solid background so the embedded icon doesn't get garbage in it
    values[1] = true; //bypass wM
    values[2] = XCB_EVENT_MASK_VISIBILITY_CHANGE | // receive visibility change, to handle KWin restart #357443
                // Redirect and handle structure (size, position) requests from the embedded window.
                XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT;
    xcb_create_window (c,                          /* connection    */
                    XCB_COPY_FROM_PARENT,          /* depth         */
                     iContainerWid,               /* window Id     */
                     screen->root,                 /* parent window */
                     0, 0,                         /* x, y          */
                     s_embedSize, s_embedSize,     /* width, height */
                     0,                            /* border_width  */
                     XCB_WINDOW_CLASS_INPUT_OUTPUT,/* class         */
                     screen->root_visual,          /* visual        */
                     mask, values);                /* masks         */

    /*
        We need the window to exist and be mapped otherwise the child won't render it's contents

        We also need it to exist in the right place to get the clicks working as GTK will check sendEvent locations to see if our window is in the right place. So even though our contents are drawn via compositing we still put this window in the right place

        We can't composite it away anything parented owned by the root window (apparently)
        Stack Under works in the non composited case, but it doesn't seem to work in kwin's composited case (probably need set relevant NETWM hint)

        As a last resort set opacity to 0 just to make sure this container never appears
    */


    stackContainerWindow(XCB_STACK_MODE_BELOW);

    NETWinInfo wm(c, iContainerWid, screen->root, NET::Properties(), NET::Properties2());
    wm.setOpacity(0);


    xcb_flush(c);

    xcb_map_window(c, iContainerWid);

    xcb_reparent_window(c, iWindowId,
                        iContainerWid,
                        0, 0);

    /*
     * Render the embedded window offscreen
     */
    xcb_composite_redirect_window(c, iWindowId, XCB_COMPOSITE_REDIRECT_MANUAL);


    /* we grab the window, but also make sure it's automatically reparented back
     * to the root window if we should die.
    */
    xcb_change_save_set(c, XCB_SET_MODE_INSERT, iWindowId);

    //tell client we're embedding it
    xembed_message_send(iWindowId, XEMBED_EMBEDDED_NOTIFY, 0, iContainerWid, XEMBED_VERSION);

    //move window we're embedding
    const uint32_t windowMoveConfigVals[2] = { 0, 0 };

    xcb_configure_window(c, iWindowId,
                             XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
                             windowMoveConfigVals);

    QSize clientWindowSize = calculateClientWindowSize();

    //show the embedded window otherwise nothing happens
    xcb_map_window(c, iWindowId);

    xcb_clear_area(c, 0, iWindowId, 0, 0, clientWindowSize.width(), clientWindowSize.height());

    xcb_flush(c);

    //guess which input injection method to use
    //we can either send an X event to the client or XTest
    //some don't support direct X events (GTK3/4), and some don't support XTest because reasons
    //note also some clients might not have the XTest extension. We may as well assume it does and just fail to send later.

    //we query if the client selected button presses in the event mask
    //if the client does supports that we send directly, otherwise we'll use xtest
    auto waCookie = xcb_get_window_attributes(c, iWindowId);
    QScopedPointer<xcb_get_window_attributes_reply_t, QScopedPointerPodDeleter> windowAttributes(xcb_get_window_attributes_reply(c, waCookie, nullptr));
    if (windowAttributes && ! (windowAttributes->all_event_masks & XCB_EVENT_MASK_BUTTON_PRESS)) {
        iInjectMode = XTest;
    }

    //there's no damage event for the first paint, and sometimes it's not drawn immediately
    //not ideal, but it works better than nothing
    //test with xchat before changing
    QTimer::singleShot(500, this, &trayicon::updateIcon);
}

void trayicon::updateIcon(){
    const QImage image = getImageNonComposite();
    if (image.isNull()) {
        qDebug() << "No xembed icon for" << iWindowId << Title();
        return;
    }

    int w = image.width();
    int h = image.height();

    iPixmap = QPixmap::fromImage(image);
    if (w > s_embedSize || h > s_embedSize) {
        qDebug() << "Scaling pixmap of window" << iWindowId << Title() << "from w*h" << w << h;
        iPixmap = iPixmap.scaled(s_embedSize, s_embedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    setIcon(QIcon(iPixmap));
    //Q_EMIT NewIcon();
    //Q_EMIT NewToolTip();
}

void trayicon::resizeWindow(const uint16_t width, const uint16_t height) const
{
    auto connection = QX11Info::connection();

    uint16_t widthNormalized = std::min(width, s_embedSize);
    uint16_t heighNormalized = std::min(height, s_embedSize);

    const uint32_t windowSizeConfigVals[2] = { widthNormalized, heighNormalized };
    xcb_configure_window(connection, iWindowId,
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                         windowSizeConfigVals);

    xcb_flush(connection);
}

void trayicon::hideContainerWindow(xcb_window_t windowId) const
{
    if (iContainerWid == windowId && !sendingClickEvent) {
        qDebug() << "Container window visible, stack below";
        stackContainerWindow(XCB_STACK_MODE_BELOW);
    }
}

QSize trayicon::calculateClientWindowSize() const
{
    auto c = QX11Info::connection();

    auto cookie = xcb_get_geometry(c, iWindowId);
    QScopedPointer<xcb_get_geometry_reply_t, QScopedPointerPodDeleter>
    clientGeom(xcb_get_geometry_reply(c, cookie, nullptr));

    QSize clientWindowSize;
    if (clientGeom) {
        clientWindowSize = QSize(clientGeom->width, clientGeom->height);
    }
    //if the window is a clearly stupid size resize to be something sensible
    //this is needed as chromium and such when resized just fill the icon with transparent space and only draw in the middle
    //however KeePass2 does need this as by default the window size is 273px wide and is not transparent
    //use an artbitrary heuristic to make sure icons are always sensible
    if (clientWindowSize.isEmpty() || clientWindowSize.width() > s_embedSize || clientWindowSize.height() > s_embedSize) {
        qDebug() << "Resizing window" << iWindowId << Title() << "from w*h" << clientWindowSize;

        resizeWindow(s_embedSize, s_embedSize);

        clientWindowSize = QSize(s_embedSize, s_embedSize);
    }

    return clientWindowSize;
}


void sni_cleanup_xcb_image(void *data) {
    xcb_image_destroy(static_cast<xcb_image_t*>(data));
}

bool trayicon::isTransparentImage(const QImage& image) const
{
    int w = image.width();
    int h = image.height();

    // check for the center and sub-center pixels first and avoid full image scan
    if (! (qAlpha(image.pixel(w >> 1, h >> 1)) + qAlpha(image.pixel(w >> 2, h >> 2)) == 0))
        return false;

    // skip scan altogether if sub-center pixel found to be opaque
    // and break out from the outer loop too on full scan
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            if (qAlpha(image.pixel(x, y))) {
                // Found an opaque pixel.
                return false;
            }
        }
    }

    return true;
}

QImage trayicon::getImageNonComposite() const
{
    auto c = QX11Info::connection();

    QSize clientWindowSize = calculateClientWindowSize();

    xcb_image_t *image = xcb_image_get(c, iWindowId, 0, 0, clientWindowSize.width(), clientWindowSize.height(), 0xFFFFFFFF, XCB_IMAGE_FORMAT_Z_PIXMAP);

    // Don't hook up cleanup yet, we may use a different QImage after all
    QImage naiveConversion;
    if (image) {
        naiveConversion = QImage(image->data, image->width, image->height, QImage::Format_ARGB32);
    } else {
        qDebug() << "Skip NULL image returned from xcb_image_get() for" << iWindowId << Title();
        return QImage();
    }

    if (isTransparentImage(naiveConversion)) {
        QImage elaborateConversion = QImage(convertFromNative(image));

        // Update icon only if it is at least partially opaque.
        // This is just a workaround for X11 bug: xembed icon may suddenly
        // become transparent for a one or few frames. Reproducible at least
        // with WINE applications.
        if (isTransparentImage(elaborateConversion)) {
            qDebug() << "Skip transparent xembed icon for" << iWindowId << Title();
            return QImage();
        } else
            return elaborateConversion;
    } else {
        // Now we are sure we can eventually delete the xcb_image_t with this version
        return QImage(image->data, image->width, image->height, image->stride, QImage::Format_ARGB32, sni_cleanup_xcb_image, image);
    }
}

QImage trayicon::convertFromNative(xcb_image_t *xcbImage) const
{
    QImage::Format format = QImage::Format_Invalid;

    switch (xcbImage->depth) {
    case 1:
        format = QImage::Format_MonoLSB;
        break;
    case 16:
        format = QImage::Format_RGB16;
        break;
    case 24:
        format = QImage::Format_RGB32;
        break;
    case 30: {
        // Qt doesn't have a matching image format. We need to convert manually
        quint32 *pixels = reinterpret_cast<quint32 *>(xcbImage->data);
        for (uint i = 0; i < (xcbImage->size / 4); i++) {
            int r = (pixels[i] >> 22) & 0xff;
            int g = (pixels[i] >> 12) & 0xff;
            int b = (pixels[i] >>  2) & 0xff;

            pixels[i] = qRgba(r, g, b, 0xff);
        }
        // fall through, Qt format is still Format_ARGB32_Premultiplied
        Q_FALLTHROUGH();
    }
    case 32:
        format = QImage::Format_ARGB32_Premultiplied;
        break;
    default:
        return QImage(); // we don't know
    }

    QImage image(xcbImage->data, xcbImage->width, xcbImage->height, xcbImage->stride, format, sni_cleanup_xcb_image, xcbImage);

    if (image.isNull()) {
        return QImage();
    }

    if (format == QImage::Format_RGB32 && xcbImage->bpp == 32)
    {
        QImage m = image.createHeuristicMask();
        QBitmap mask(QPixmap::fromImage(m));
        QPixmap p = QPixmap::fromImage(image);
        p.setMask(mask);
        image = p.toImage();
    }

    // work around an abort in QImage::color
    if (image.format() == QImage::Format_MonoLSB) {
        image.setColorCount(2);
        image.setColor(0, QColor(Qt::white).rgb());
        image.setColor(1, QColor(Qt::black).rgb());
    }

    return image;
}


/*
  Wine is using XWindow Shape Extension for transparent tray icons.
  We need to find first clickable point starting from top-left.
*/
QPoint trayicon::calculateClickPoint() const
{
    QPoint clickPoint = QPoint(0, 0);

    auto c = QX11Info::connection();

    // request extent to check if shape has been set
    xcb_shape_query_extents_cookie_t extentsCookie = xcb_shape_query_extents(c, iWindowId);
    // at the same time make the request for rectangles (even if this request isn't needed)
    xcb_shape_get_rectangles_cookie_t rectaglesCookie = xcb_shape_get_rectangles(c, iWindowId, XCB_SHAPE_SK_BOUNDING);

    QScopedPointer<xcb_shape_query_extents_reply_t, QScopedPointerPodDeleter>
        extentsReply(xcb_shape_query_extents_reply(c, extentsCookie, nullptr));
    QScopedPointer<xcb_shape_get_rectangles_reply_t, QScopedPointerPodDeleter>
        rectanglesReply(xcb_shape_get_rectangles_reply(c, rectaglesCookie, nullptr));

    if (!extentsReply || !rectanglesReply || !extentsReply->bounding_shaped) {
        return clickPoint;
    }

    xcb_rectangle_t *rectangles = xcb_shape_get_rectangles_rectangles(rectanglesReply.get());
    if (!rectangles) {
        return clickPoint;
    }

    const QImage image = getImageNonComposite();

    double minLength = sqrt(pow(image.height(), 2) + pow(image.width(), 2));
    const int nRectangles = xcb_shape_get_rectangles_rectangles_length(rectanglesReply.get());
    for (int i = 0; i < nRectangles; ++i) {
        double length = sqrt(pow(rectangles[i].x, 2) + pow(rectangles[i].y, 2));
        if (length < minLength) {
            minLength = length;
            clickPoint = QPoint(rectangles[i].x, rectangles[i].y);
        }
    }

    qDebug() << "Click point:" << clickPoint;
    return clickPoint;
}

void trayicon::stackContainerWindow(const uint32_t stackMode) const{
    auto c = QX11Info::connection();
    const uint32_t stackData[] = {stackMode};
    xcb_configure_window(c, iContainerWid, XCB_CONFIG_WINDOW_STACK_MODE, stackData);
}

QString trayicon::Title() const{
    return Xcbutills::getWindowTitle(iWindowId);
}

// Actions =========================
//TODO: handle scroll
void trayicon::handleClick(QMouseEvent *event){
    int x = event->pos().x();
    int y = event->pos().y();

    if (event->button() == Qt::LeftButton)
        sendClick(XCB_BUTTON_INDEX_1, x, y);
    else if (event->button() == Qt::MiddleButton)
        sendClick(XCB_BUTTON_INDEX_2, x, y);
    else if (event->button() == Qt::RightButton)
        sendClick(XCB_BUTTON_INDEX_3, x, y);
}

void trayicon::Scroll(int delta, const QString& orientation){
    if (orientation == QLatin1String("vertical")) {
        sendClick(delta > 0 ? XCB_BUTTON_INDEX_4: XCB_BUTTON_INDEX_5, 0, 0);
    } else {
        sendClick(delta > 0 ? 6: 7, 0, 0);
    }
}

void trayicon::sendClick(uint8_t mouseButton, int x, int y)
{
    //it's best not to look at this code
    //GTK doesn't like send_events and double checks the mouse position matches where the window is and is top level
    //in order to solve this we move the embed container over to where the mouse is then replay the event using send_event
    //if patching, test with xchat + xchat context menus

    //note x,y are not actually where the mouse is, but the plasmoid
    //ideally we should make this match the plasmoid hit area

    qDebug() << "Received click" << mouseButton << "with passed x*y" << x << y;
    sendingClickEvent = true;

    auto c = QX11Info::connection();

    auto cookieSize = xcb_get_geometry(c, iWindowId);
    QScopedPointer<xcb_get_geometry_reply_t, QScopedPointerPodDeleter>
        clientGeom(xcb_get_geometry_reply(c, cookieSize, nullptr));

    if (!clientGeom) {
        return;
    }

    auto cookie = xcb_query_pointer(c, iWindowId);
    QScopedPointer<xcb_query_pointer_reply_t, QScopedPointerPodDeleter>
        pointer(xcb_query_pointer_reply(c, cookie, nullptr));
    /*qCDebug(SNIPROXY) << "samescreen" << pointer->same_screen << endl
    << "root x*y" << pointer->root_x << pointer->root_y << endl
    << "win x*y" << pointer->win_x << pointer->win_y;*/

    //move our window so the mouse is within its geometry
    uint32_t configVals[2] = {0, 0};
    const QPoint clickPoint = calculateClickPoint();
    if (mouseButton >= XCB_BUTTON_INDEX_4) {
    //scroll event, take pointer position
    configVals[0] = pointer->root_x;
    configVals[1] = pointer->root_y;
    } else {
    if (pointer->root_x > x + clientGeom->width)
        configVals[0] = pointer->root_x - clientGeom->width + 1;
    else
        configVals[0] = static_cast<uint32_t>(x - clickPoint.x());
    if (pointer->root_y > y + clientGeom->height)
        configVals[1] = pointer->root_y - clientGeom->height + 1;
    else
        configVals[1] = static_cast<uint32_t>(y - clickPoint.y());
    }
    xcb_configure_window(c, iContainerWid, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, configVals);

    //pull window up
    stackContainerWindow(XCB_STACK_MODE_ABOVE);

    //mouse down
    if (iInjectMode == Direct) {
        xcb_button_press_event_t* event = new xcb_button_press_event_t;
        memset(event, 0x00, sizeof(xcb_button_press_event_t));
        event->response_type = XCB_BUTTON_PRESS;
        event->event = iWindowId;
        event->time = QX11Info::getTimestamp();
        event->same_screen = 1;
        event->root = QX11Info::appRootWindow();
        event->root_x = x;
        event->root_y = y;
        event->event_x = static_cast<int16_t>(clickPoint.x());
        event->event_y = static_cast<int16_t>(clickPoint.y());
        event->child = 0;
        event->state = 0;
        event->detail = mouseButton;

        xcb_send_event(c, false, iWindowId, XCB_EVENT_MASK_BUTTON_PRESS, (char *) event);
        delete event;
    } else {
//        sendXTestPressed(QX11Info::display(), mouseButton);
    }

    //mouse up
    if (iInjectMode == Direct)
    {
        xcb_button_release_event_t* event = new xcb_button_release_event_t;
        memset(event, 0x00, sizeof(xcb_button_release_event_t));
        event->response_type = XCB_BUTTON_RELEASE;
        event->event = iWindowId;
        event->time = QX11Info::getTimestamp();
        event->same_screen = 1;
        event->root = QX11Info::appRootWindow();
        event->root_x = x;
        event->root_y = y;
        event->event_x = static_cast<int16_t>(clickPoint.x());
        event->event_y = static_cast<int16_t>(clickPoint.y());
        event->child = 0;
        event->state = 0;
        event->detail = mouseButton;

        xcb_send_event(c, false, iWindowId, XCB_EVENT_MASK_BUTTON_RELEASE, (char *) event);
        delete event;
    } else {
//        sendXTestReleased(QX11Info::display(), mouseButton);
    }

    stackContainerWindow(XCB_STACK_MODE_BELOW);


    sendingClickEvent = false;
}
