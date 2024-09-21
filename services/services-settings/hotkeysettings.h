/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2024 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef HOTKEYSETTINGS_H
#define HOTKEYSETTINGS_H

#include <QObject>

#include "../../library/pluginutills/settings_plugin_interface.h"

#include "edithotkeywidget.h"

class HotkeySettingItem : public QObject{
    Q_OBJECT

public:
    HotkeySettingItem(QString id) : item_id(id) {}

signals:
    void item_changed();

public slots:
    void edit();
    void remove();

private slots:
    void save(const HotkeyData& data);

private:
    QString item_id;
    edithotkeywidget* edit_widget = nullptr;
};

class HotkeySettings : public QObject{
    Q_OBJECT
public:
    HotkeySettings();

public slots:
    settings_category* get_settings_item(){ return settings_item; }

signals:

private slots:
    void load_hotkeys();
    void reload_hotkeys();
    void add_item();

private:
    settings_category *settings_item = nullptr;
    settings_widget_group *hotkey_widget_group = nullptr;
    QList<HotkeySettingItem*> item_list;
};

#endif // HOTKEYSETTINGS_H
