// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CPUMONITORSETTINGS_H
#define CPUMONITORSETTINGS_H

#include "settings_plugin_interface.h"

class CpuMonitorSettings : public QObject, settings_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.cpumonitor.plugin")
    Q_INTERFACES(settings_plugin_interface)

public:
    QList<settings_page*> pages() override;
};

#endif // CPUMONITORSETTINGS_H
