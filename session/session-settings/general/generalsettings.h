// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <QObject>
#include <QLineEdit>
#include <QCheckBox>

#include "settings_plugin_interface.h"

class GeneralSettings : public QObject {
    Q_OBJECT
public:
    GeneralSettings();
    settings_category* get_settings_item() { return settings_item; }
private slots:
    void load_settings();
    void save_settings();
private:
    settings_category *settings_item = nullptr;
    QLineEdit *wm_edit = nullptr;
    QCheckBox *xdg_checkbox = nullptr;
};

#endif // GENERALSETTINGS_H
