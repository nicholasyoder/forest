// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef APP_PLUGIN_INTERFACE_H
#define APP_PLUGIN_INTERFACE_H

#include <QObject>

class app_plugin_interface {

public:

    // Destructor
    virtual ~app_plugin_interface() {}

    // Called soon after plugin constuctor runs
    virtual void setupPlug() = 0;

};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(app_plugin_interface, "forest.app.plugin.interface")
QT_END_NAMESPACE


#endif // APP_PLUGIN_INTERFACE_H
