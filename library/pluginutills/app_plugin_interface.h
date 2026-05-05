// SPDX-License-Identifier: LGPL-3.0-or-later

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
