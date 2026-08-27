// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SERVICES_H
#define SERVICES_H

#include <QObject>
#include <QtDBus>
#include <QLayout>

#include "../../library/pluginutills/app_plugin_interface.h"
#include "hotkeys/foresthotkeys.h"
#include "notifications/notify.h"
#include "polkit/polkitagent.h"

class services : public QObject, app_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.app.services.plugin")
    Q_INTERFACES(app_plugin_interface)

public:
    services();

    //begin pluginterface
    void setupPlug();
    void XcbEventFilter(xcb_generic_event_t*){}
    bool needs_xcb_events(){ return false; }
    //end pluginterface

private:
    foresthotkeys *fhotkeys;
    notify *fnotify;
    polkitagent *fpolkit;
};

#endif // SERVICES_H
