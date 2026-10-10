// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOCKERSETTINGSPLUGIN_H
#define LOCKERSETTINGSPLUGIN_H

#include "settings_plugin_interface.h"

// "Lock Screen" page for forest-locker's LockerConfig.
class LockerSettingsPlugin : public QObject, settings_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.locker.plugin")
    Q_INTERFACES(settings_plugin_interface)

public:
    QList<settings_page*> pages() override;
};

#endif // LOCKERSETTINGSPLUGIN_H
