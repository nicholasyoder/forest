// SPDX-License-Identifier: LGPL-3.0-or-later

#include "autostartsettings.h"

#include <QSettings>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

#include "miscutills.h"

AutostartSettings::AutostartSettings() {
    settings_item = new settings_category("Autostart", "", "preferences-system-session");
    connect(settings_item, &settings_category::opened, this, &AutostartSettings::load_settings);
}

void AutostartSettings::load_settings() {
    autostart_widget_group = new settings_widget_group;
    settings_item->add_child(autostart_widget_group);

    QSettings settings("Forest", "Session");
    settings.beginGroup("Autostart");

    foreach(QString key, settings.childGroups()) {
        settings.beginGroup(key);
        QString command = settings.value("command", "").toString();
        settings.endGroup();

        AutostartSettingItem *item = new AutostartSettingItem(key);
        connect(item, &AutostartSettingItem::item_changed, this, &AutostartSettings::reload_settings);

        QWidget *row_widget = new QWidget;
        QHBoxLayout *h_layout = new QHBoxLayout(row_widget);
        h_layout->setContentsMargins(0, 0, 0, 0);

        QLineEdit *cmd_edit = new QLineEdit(command);
        h_layout->addWidget(cmd_edit);

        QPushButton *remove_bt = new QPushButton(QIcon::fromTheme("edit-delete"), "");
        remove_bt->setToolTip("Remove");
        connect(remove_bt, &QPushButton::clicked, item, &AutostartSettingItem::remove);
        h_layout->addWidget(remove_bt);

        RunOnce *runner = new RunOnce(1000);
        connect(cmd_edit, &QLineEdit::textChanged, runner, &RunOnce::try_activate);
        connect(runner, &RunOnce::activated, this, [item, cmd_edit](){
            item->save(cmd_edit->text());
        });

        settings_widget *entry = new settings_widget("", "", row_widget);
        autostart_widget_group->add_child(entry);
    }

    QPushButton *add_bt = new QPushButton(QIcon::fromTheme("list-add"), "");
    add_bt->setToolTip("Add");
    connect(add_bt, &QPushButton::clicked, this, &AutostartSettings::add_item);
    settings_widget *add_entry = new settings_widget("Add command", "", add_bt);
    autostart_widget_group->add_child(add_entry);
}

void AutostartSettings::reload_settings() {
    settings_item->clear();
    load_settings();
    settings_item->notify_updated();
}

void AutostartSettings::add_item() {
    QSettings settings("Forest", "Session");
    settings.beginGroup("Autostart");

    QStringList groups = settings.childGroups();
    int new_number = 1;
    if (!groups.isEmpty())
        new_number = groups.last().split("-").last().toInt() + 1;

    settings.setValue("item-" + miscutills::pad_with_zeros(new_number) + "/command", "");
    settings.sync();

    reload_settings();
}

void AutostartSettingItem::save(const QString &command) {
    QSettings settings("Forest", "Session");
    settings.beginGroup("Autostart/" + item_id);
    settings.setValue("command", command);
    settings.sync();
}

void AutostartSettingItem::remove() {
    QSettings settings("Forest", "Session");
    settings.beginGroup("Autostart");

    settings.beginGroup(item_id);
    settings.remove("");
    settings.endGroup();
    settings.remove(item_id);

    int i = 1;
    foreach(QString key, settings.childGroups()) {
        int item_number = key.split("-").last().toInt();
        if (item_number != i) {
            QString new_key = "item-" + miscutills::pad_with_zeros(i);
            settings.beginGroup(key);
            QStringList subkeys = settings.childKeys();
            settings.endGroup();
            foreach(QString subkey, subkeys)
                settings.setValue(new_key + "/" + subkey, settings.value(key + "/" + subkey));
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
