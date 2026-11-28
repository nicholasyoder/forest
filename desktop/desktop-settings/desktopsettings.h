// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKTOPSETTINGS_H
#define DESKTOPSETTINGS_H

#include <QObject>

#include "../../library/pluginutills/settings_plugin_interface.h"

class DesktopSettings : public QObject, settings_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.desktop.plugin")
    Q_INTERFACES(settings_plugin_interface)

public:
    DesktopSettings();
    ~DesktopSettings();

    QList<settings_page*> pages() override;

private:
    settings_page *make_wallpaper_page();
    settings_page *make_icons_page();
};

#endif // DESKTOPSETTINGS_H
