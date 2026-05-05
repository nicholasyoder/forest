// SPDX-License-Identifier: LGPL-3.0-or-later

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
