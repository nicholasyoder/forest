// SPDX-License-Identifier: LGPL-3.0-or-later

#include "hotkeysettings.h"

#include <QSettings>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "miscutills.h"

HotkeySettings::HotkeySettings(){
    settings_item = new settings_category("Hotkeys", "", "preferences-desktop-keyboard");
    connect(settings_item, &settings_category::opened, this, &HotkeySettings::refresh);
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
        h_layout->setContentsMargins(QMargins(0,0,0,0));
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

void HotkeySettings::refresh(){
    settings_item->clear();
    // Later: the sender may be one of them, mid-emit.
    for (HotkeySettingItem *item : std::as_const(item_list)) item->deleteLater();
    item_list.clear();
    load_hotkeys();
    settings_item->notify_updated();
}

void HotkeySettings::reload_hotkeys(){
    refresh();
    miscutills::call_dbus("forest/hotkeys/reloadhotkeys");
}

void HotkeySettings::add_item(){
    QSettings settings("Forest", "Forest");
    settings.beginGroup("hotkeys");

    const QStringList items = settings.childGroups();
    int item_numer = items.isEmpty() ? 0 : items.last().split("-").last().toInt();
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
    data.action = settings.value("action", "").toString();

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
    settings.setValue("action", data.action);
    settings.sync();
    emit item_changed();
}
