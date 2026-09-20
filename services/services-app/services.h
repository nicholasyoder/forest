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
#include "systemtray/statusnotifierwatcher.h"

class services : public QObject, app_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.app.services.plugin")
    Q_INTERFACES(app_plugin_interface)

public:
    services();

    //begin pluginterface
    void setupPlug();
    //end pluginterface

private:
    foresthotkeys *fhotkeys;
    notify *fnotify;
    polkitagent *fpolkit;
    StatusNotifierWatcher *fsystemtray;
};

#endif // SERVICES_H
