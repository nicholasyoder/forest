// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ICONTHEMESETTINGS_H
#define ICONTHEMESETTINGS_H

#include <QObject>

#include "listwidget.h"
#include "../../library/pluginutills/settings_plugin_interface.h"

class IconThemeSettings : public QObject
{
    Q_OBJECT
public:
    IconThemeSettings();

public slots:
    settings_page *get_settings_item() { return settings_item; }

private slots:
    void load_icon_themes();
    void set_icon_theme(QListWidgetItem *item);

private:
    settings_page *settings_item = nullptr;
    ListWidget *icon_theme_list = nullptr;
};

#endif // ICONTHEMESETTINGS_H
