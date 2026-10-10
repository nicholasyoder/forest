// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WINDOWLISTSETTINGS_H
#define WINDOWLISTSETTINGS_H

#include "settings_plugin_interface.h"

class WindowListSettings : public QObject, settings_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.windowlist.plugin")
    Q_INTERFACES(settings_plugin_interface)

public:
    QList<settings_page*> pages() override;
};

#endif // WINDOWLISTSETTINGS_H
