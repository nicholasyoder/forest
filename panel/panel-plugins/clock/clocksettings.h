// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CLOCKSETTINGS_H
#define CLOCKSETTINGS_H

#include "settings_plugin_interface.h"

class ClockSettings : public QObject, settings_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.clock.plugin")
    Q_INTERFACES(settings_plugin_interface)

public:
    QList<settings_page*> pages() override;
};

#endif // CLOCKSETTINGS_H
