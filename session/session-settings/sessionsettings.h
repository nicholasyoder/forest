// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SESSIONSETTINGS_H
#define SESSIONSETTINGS_H

#include "settings_plugin_interface.h"
#include "autostart/autostartsettings.h"
#include "general/generalsettings.h"

class SessionSettings : public QObject, settings_plugin_infterace
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.session.plugin")
    Q_INTERFACES(settings_plugin_infterace)

public:
    SessionSettings();
    ~SessionSettings();

    QList<settings_item*> get_settings_items() override;

private:
    GeneralSettings *general_settings = nullptr;
    AutostartSettings *autostart_settings = nullptr;
};

#endif // SESSIONSETTINGS_H
