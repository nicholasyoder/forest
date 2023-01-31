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

#ifndef XCBUTILLS_H
#define XCBUTILLS_H

#include <QString>
#include <QDebug>
#include <QIcon>
#include <QApplication>
#include <QScreen>
#include <QtX11Extras/QX11Info>
#include <xcb/xcb.h>

/** XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY          0
#define XEMBED_WINDOW_ACTIVATE          1
#define XEMBED_WINDOW_DEACTIVATE        2
#define XEMBED_REQUEST_FOCUS            3
#define XEMBED_FOCUS_IN                 4
#define XEMBED_FOCUS_OUT                5
#define XEMBED_FOCUS_NEXT               6
#define XEMBED_FOCUS_PREV               7

class Xcbutills
{
public:
    Xcbutills(){}

    //get atom by name
    static xcb_atom_t atom(QString name);

    //return the name of an atom
    static char* getAtomName(xcb_atom_t atom);

    //returns a list of open windows
    static QList<xcb_window_t> getClientList();

    //check if window should be shown in the taskbar
    static bool isWindow4Taskbar(xcb_window_t window);

    //gets the WM_NAME window property
    static QString getWindowTitle(xcb_window_t window);

    //return the window icon
    static QIcon getWindowIcon(xcb_window_t window);

    //get which desktop a window is on
    static int getWindowDesktop(xcb_window_t window);

    static QImage getWindowImage(xcb_window_t window);

    static int getNumDesktops();

    //get the active desktop
    static int getCurrentDesktop();

    //show/unshow desktop
    static void showDesktop();

    //get active window
    static xcb_window_t getActiveWindow();

    //activate window
    static void raiseWindow(xcb_window_t window);

    static void maximizeWindow(xcb_window_t window);

    static void demaximizeWindow(xcb_window_t window);

    static void minimizeWindow(xcb_window_t window);

    static void closeWindow(xcb_window_t window);

    static void resizeWindow(xcb_window_t window, int w, int h);

    static void moveWindow(xcb_window_t window, int x, int y);

    //Move window so top of window (title bar) is on screen
    static void fitWindowOnScreen(xcb_window_t window);

    static void setCurrentDesktop(int desknum);

    //not sure what this actually does -- setPartialStrut seems to be what should be used
    //static void setStrut(xcb_window_t window, QRect strut);

    static void setPartialStrut(xcb_window_t window, int left_width, int right_width, int top_width, int bottom_width,
                                int left_start, int left_end, int right_start, int right_end, int top_start, int top_end, int bottom_start, int bottom_end);

    //from kwindowsystem - mostly unmodified ---------------------------------------------------------------------------------------------------
    template <typename T>
    static T get_value_reply(xcb_connection_t *c, const xcb_get_property_cookie_t cookie, xcb_atom_t type, T def, bool *success = nullptr)
    {
        T value = def;
        xcb_get_property_reply_t *reply = xcb_get_property_reply(c, cookie, nullptr);
        if (success)
            *success = false;

        if (reply){
            if (reply->type == type && reply->value_len == 1 && reply->format == sizeof(T) * 8){
                value = *reinterpret_cast<T *>(xcb_get_property_value(reply));
                if (success)
                    *success = true;
            }
            free(reply);
        }

        return value;
    }

    template <typename T>
    static QVector<T> get_array_reply(xcb_connection_t *c, const xcb_get_property_cookie_t cookie, xcb_atom_t type)
    {
        xcb_get_property_reply_t *reply = xcb_get_property_reply(c, cookie, nullptr);
        if (!reply)
            return QVector<T>();

        QVector<T> vector;
        if (reply->type == type && reply->value_len > 0 && reply->format == sizeof(T) * 8){
            T *data = reinterpret_cast<T *>(xcb_get_property_value(reply));
            vector.resize(int(reply->value_len));
            memcpy(static_cast<void *>(&vector.first()), static_cast<void *>(data), reply->value_len * sizeof(T));
        }

        free(reply);
        return vector;
    }

    static QByteArray get_string_reply(xcb_connection_t *c, const xcb_get_property_cookie_t cookie, xcb_atom_t type)
    {
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

    static char *nstrndup(const char *s1, int l)
    {
        if (! s1 || l == 0) {
            return static_cast<char *>(nullptr);
        }

        char *s2 = new char[ulong(l + 1)];
        strncpy(s2, s1, ulong(l));
        s2[l] = '\0';
        return s2;
    }

    static void send_client_message(xcb_connection_t *c, uint32_t mask, xcb_window_t destination, xcb_window_t window, xcb_atom_t message, const uint32_t data[])
    {
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
    //from kwindowsystem - mostly unmodified ---------------------------------------------------------------------------------------------------

    struct xicon {
        xicon(QSize isize, uchar *idata){size = isize; data = idata;}

        QSize size = QSize(0,0);

        //Image data for the icon.  This is an array of 32bit packed CARDINAL ARGB
        //with high byte being A, low byte being B. First two bytes are width, height.
        //Data is in rows, left to right and top to bottom.
        uchar *data = nullptr;
    };

    static QVector<xicon> readxicon(xcb_connection_t *c, const xcb_get_property_cookie_t cookie)
    {
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

    static xcb_connection_t* xcbconnection;
};

#endif // XCBUTILLS_H
