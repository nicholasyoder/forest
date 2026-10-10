// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MEMORYMONITORSETTINGS_H
#define MEMORYMONITORSETTINGS_H

#include "settings_plugin_interface.h"

class MemoryMonitorSettings : public QObject, settings_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.memorymonitor.plugin")
    Q_INTERFACES(settings_plugin_interface)

public:
    QList<settings_page*> pages() override;
};

#endif // MEMORYMONITORSETTINGS_H
