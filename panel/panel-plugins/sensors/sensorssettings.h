// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SENSORSSETTINGS_H
#define SENSORSSETTINGS_H

#include "settings_plugin_interface.h"

class Sensors;

class SensorsSettings : public QObject, settings_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.sensors.plugin")
    Q_INTERFACES(settings_plugin_interface)

public:
    ~SensorsSettings() override;
    QList<settings_page*> pages() override;

private:
    Sensors *sensors = nullptr;
};

#endif // SENSORSSETTINGS_H
