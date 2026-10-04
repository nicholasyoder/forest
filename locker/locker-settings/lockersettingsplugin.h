// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOCKERSETTINGSPLUGIN_H
#define LOCKERSETTINGSPLUGIN_H

#include <QCheckBox>
#include <QSpinBox>

#include "settings_plugin_interface.h"

// "Lock Screen" panel for forest-locker's LockerConfig.
class LockerSettingsPlugin : public QObject, settings_plugin_infterace
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.locker.plugin")
    Q_INTERFACES(settings_plugin_infterace)

public:
    QList<settings_item*> get_settings_items() override;

private slots:
    void load_settings();
    void save_settings();

private:
    QSpinBox *display_off_spin = nullptr;
    QCheckBox *lock_on_display_off_check = nullptr;
    QCheckBox *dim_check = nullptr;
    QCheckBox *lock_on_suspend_check = nullptr;
    QSpinBox *locked_display_off_spin = nullptr;
    bool loading = false;
};

#endif // LOCKERSETTINGSPLUGIN_H
