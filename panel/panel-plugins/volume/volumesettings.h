// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VOLUMESETTINGS_H
#define VOLUMESETTINGS_H

#include "settings_plugin_interface.h"

class VolumeSettings : public QObject, settings_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.volume.plugin")
    Q_INTERFACES(settings_plugin_interface)

public:
    QList<settings_page*> pages() override;
};

#endif // VOLUMESETTINGS_H
