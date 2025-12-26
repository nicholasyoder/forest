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

#include "hotkeysettings.h"

#include <QSettings>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "miscutills.h"

HotkeySettings::HotkeySettings(){
    settings_item = new settings_category("Hotkeys", "", "preferences-desktop-keyboard");
    connect(settings_item, &settings_category::opened, this, &HotkeySettings::load_hotkeys);
}

void HotkeySettings::load_hotkeys(){
    hotkey_widget_group = new settings_widget_group;
    settings_item->add_child(hotkey_widget_group);

    QSettings settings("Forest", "Forest");
    settings.beginGroup("hotkeys");
    foreach(QString key, settings.childGroups()){
        settings.beginGroup(key);
        HotkeySettingItem* item = new HotkeySettingItem(key);
        connect(item, &HotkeySettingItem::item_changed, this, &HotkeySettings::reload_hotkeys);
        item_list.append(item);

        QWidget *item_widget = new QWidget;
        QHBoxLayout *h_layout = new QHBoxLayout(item_widget);
        h_layout->setMargin(0);
        h_layout->addSpacing(5);
        QLabel* keys_label = new QLabel(settings.value("keysequence", "").toString());
        keys_label->setStyleSheet("color: grey;");
        h_layout->addWidget(keys_label);
        h_layout->addSpacing(5);
        QPushButton* edit_bt = new QPushButton(QIcon::fromTheme("edit"), "");
        connect(edit_bt, &QPushButton::clicked, item, &HotkeySettingItem::edit);
        edit_bt->setToolTip("Edit");
        h_layout->addWidget(edit_bt);
        QPushButton* remove_bt = new QPushButton(QIcon::fromTheme("edit-delete"), "");
        connect(remove_bt, &QPushButton::clicked, item, &HotkeySettingItem::remove);
        remove_bt->setToolTip("Remove");
        h_layout->addWidget(remove_bt);
        settings_widget *hotkey_item = new settings_widget(settings.value("description").toString(),"", item_widget);
        hotkey_widget_group->add_child(hotkey_item);

        settings.endGroup();
    }

    QPushButton* add_bt = new QPushButton(QIcon::fromTheme("edit-add"), "");
    add_bt->setToolTip("Add");
    settings_widget *hotkey_item = new settings_widget("Add hotkey","", add_bt);
    connect(add_bt, &QPushButton::clicked, this, &HotkeySettings::add_item);
    hotkey_widget_group->add_child(hotkey_item);
}

void HotkeySettings::reload_hotkeys(){
    settings_item->clear();
    load_hotkeys();
    settings_item->notify_updated();
    miscutills::call_dbus("forest/hotkeys/reloadhotkeys");
}

void HotkeySettings::add_item(){
    QSettings settings("Forest", "Forest");
    settings.beginGroup("hotkeys");

    QString last_item = settings.childGroups().last();
    int item_numer = last_item.split("-").last().toInt();
    QString new_number = miscutills::pad_with_zeros(item_numer + 1);

    HotkeySettingItem* item = new HotkeySettingItem("item-" + new_number);
    connect(item, &HotkeySettingItem::item_changed, this, &HotkeySettings::reload_hotkeys);
    item->edit();
}

void HotkeySettingItem::edit(){
    if(!edit_widget) {
        edit_widget = new edithotkeywidget();
        connect(edit_widget, &edithotkeywidget::data_updated, this, &HotkeySettingItem::save);
    }

    QSettings settings("Forest", "Forest");
    settings.beginGroup("hotkeys/" + item_id);

    HotkeyData data;
    data.shortcut = settings.value("keysequence", "").toString();
    data.description = settings.value("description", "").toString();
    QString action = settings.value("action", "").toString();
    if(action.startsWith("DBUS:")){
        action.remove("DBUS:");
        QHash<QString, QString> dbus_options;
        foreach (QString s, action.split(",")){
            QStringList keyvalue = s.split("=");
            if (keyvalue.length() == 2) dbus_options[keyvalue.first()] = keyvalue.last();
        }
        data.action = QSharedPointer<HotkeyAction>(new CustomDBusAction(
            dbus_options["service"],
            dbus_options["path"],
            dbus_options["interface"],
            dbus_options["method"],
            dbus_options["bus"] == "System"
        ));
    }
    else {
        data.action = QSharedPointer<HotkeyAction>(new CommandAction(action));
    }

    edit_widget->set_data(data);
    edit_widget->show();
}

void HotkeySettingItem::remove(){
    QSettings settings("Forest", "Forest");
    settings.beginGroup("hotkeys");

    settings.beginGroup(item_id);
    settings.remove("");
    settings.endGroup();
    settings.remove(item_id);

    // Re-number keys
    int i = 1;
    foreach(QString key, settings.childGroups()){
        int item_numer = key.split("-").last().toInt();
        if(item_numer != i){
            QString new_key = "item-" + miscutills::pad_with_zeros(i);
            settings.beginGroup(key);
            QStringList subkeys = settings.childKeys();
            settings.endGroup();
            foreach(QString subkey, subkeys){
                settings.setValue(new_key + "/" + subkey, settings.value(key + "/" + subkey));
            }
            settings.beginGroup(key);
            settings.remove("");
            settings.endGroup();
            settings.remove(key);
        }
        i++;
    }
    settings.sync();
    emit item_changed();
}

void HotkeySettingItem::save(const HotkeyData &data){
    QSettings settings("Forest", "Forest");
    settings.beginGroup("hotkeys/" + item_id);
    settings.setValue("description", data.description);
    settings.setValue("keysequence", data.shortcut);
    QString action;
    if (auto command_action = data.action.dynamicCast<CommandAction>()) {
        action = command_action->command;
    }
    else if (auto dbus_action = data.action.dynamicCast<CustomDBusAction>()) {
        action.append("DBUS:");
        QHash<QString, QString>dbus_options;
        dbus_options["bus"] = dbus_action->isSystemBus ? "System" : "Session";
        dbus_options["service"] = dbus_action->service;
        dbus_options["path"] = dbus_action->path;
        dbus_options["interface"] = dbus_action->interface;
        dbus_options["method"] = dbus_action->method;
        foreach(QString key, dbus_options.keys()){
            action.append(key + "=" + dbus_options[key] + ",");
        }
        action.chop(1); // remove trailing comma
    }
    settings.setValue("action", action);
    settings.sync();
    emit item_changed();
}
