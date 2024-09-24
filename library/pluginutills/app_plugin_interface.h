/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2024 Nicholas Yoder
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

#ifndef APP_PLUGIN_INTERFACE_H
#define APP_PLUGIN_INTERFACE_H

#include <QObject>
#include <xcb/xcb.h>

class app_plugin_interface {

public:

    // Destructor
    virtual ~app_plugin_interface() {}

    // Called soon after plugin constuctor runs
    virtual void setupPlug() = 0;

    // Pass xcb events on to plugins
    virtual void XcbEventFilter(xcb_generic_event_t* /*event*/) = 0;

    // Get needs xcb events bool
    virtual bool needs_xcb_events() = 0;

};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(app_plugin_interface, "forest.app.plugin.interface")
QT_END_NAMESPACE


#endif // APP_PLUGIN_INTERFACE_H
