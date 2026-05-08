// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef AUTOSTARTSETTINGS_H
#define AUTOSTARTSETTINGS_H

#include <QObject>
#include <QString>

#include "settings_plugin_interface.h"

class AutostartSettingItem : public QObject {
    Q_OBJECT
public:
    AutostartSettingItem(QString id) : item_id(id) {}
signals:
    void item_changed();
public slots:
    void remove();
    void save(const QString &command);
private:
    QString item_id;
};

class AutostartSettings : public QObject {
    Q_OBJECT
public:
    AutostartSettings();
    settings_category* get_settings_item() { return settings_item; }
private slots:
    void load_settings();
    void reload_settings();
    void add_item();
private:
    settings_category *settings_item = nullptr;
    settings_widget_group *autostart_widget_group = nullptr;
};

#endif // AUTOSTARTSETTINGS_H
