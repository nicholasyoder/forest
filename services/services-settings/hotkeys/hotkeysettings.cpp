// SPDX-License-Identifier: LGPL-3.0-or-later

#include "hotkeysettings.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QSettings>
#include <QTimer>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "hotkeyconfig.h"
#include "miscutills.h"

HotkeySettings::HotkeySettings(){
    settings_item = new settings_page("input/hotkeys", "Hotkeys", "preferences-desktop-keyboard");
    settings_item->set_keywords({"keyboard", "shortcuts", "keybindings", "global"});
    connect(settings_item, &settings_category::opened, this, &HotkeySettings::refresh);
}

void HotkeySettings::load_hotkeys(){
    hotkey_widget_group = new settings_widget_group;
    settings_item->add_child(hotkey_widget_group);

    const QStringList unbound = unbound_ids();
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
        if (unbound.contains(key)){
            QLabel* warning_label = new QLabel;
            warning_label->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(16, 16));
            warning_label->setToolTip("Not bound: the compositor refused this shortcut or another one already uses it.");
            h_layout->addWidget(warning_label);
        }
        QLabel* keys_label = new QLabel(hotkeyconfig::triggerDisplay(hotkeyconfig::readTrigger(settings)));
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

// Ids the shell couldn't bind a trigger for; empty if it isn't running.
QStringList HotkeySettings::unbound_ids() const{
    QDBusMessage call = QDBusMessage::createMethodCall("org.forest", "/org/forest/hotkeys", QString(), "unboundHotkeys");
    const QDBusMessage reply = QDBusConnection::sessionBus().call(call, QDBus::Block, 500);
    return reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()
        ? reply.arguments().first().toStringList() : QStringList();
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
    // The rebind is async; show what it left unbound once it's done.
    QTimer::singleShot(1500, this, &HotkeySettings::refresh);
}

void HotkeySettings::add_item(){
    // The id is assigned at save, so two pending adds can't collide.
    HotkeySettingItem* item = new HotkeySettingItem(QString());
    connect(item, &HotkeySettingItem::item_changed, this, &HotkeySettings::reload_hotkeys);
    item_list.append(item);
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
    data.trigger = hotkeyconfig::readTrigger(settings);
    data.description = settings.value("description", "").toString();
    data.action = settings.value("action", "").toString();
    data.repeat = settings.value(hotkeyconfig::kRepeatKey, false).toBool();
    data.repeatInterval = settings.value(hotkeyconfig::kRepeatIntervalKey,
                                         hotkeyconfig::kDefaultRepeatIntervalMs).toInt();

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
    if (item_id.isEmpty()){
        settings.beginGroup("hotkeys");
        int last = 0;
        foreach(QString key, settings.childGroups()) last = qMax(last, key.split("-").last().toInt());
        settings.endGroup();
        item_id = "item-" + miscutills::pad_with_zeros(last + 1);
    }
    settings.beginGroup("hotkeys/" + item_id);
    settings.setValue("description", data.description);
    settings.setValue(hotkeyconfig::kTriggerKey, data.trigger);
    settings.remove(hotkeyconfig::kLegacyKeysequenceKey);
    settings.setValue("action", data.action);
    settings.setValue(hotkeyconfig::kRepeatKey, data.repeat);
    settings.setValue(hotkeyconfig::kRepeatIntervalKey, data.repeatInterval);
    settings.sync();
    emit item_changed();
}
