// SPDX-License-Identifier: LGPL-3.0-or-later

#include "notificationssettings.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QPushButton>
#include <QSpinBox>

#include "../../notificationsconfig.h"
#include "settingsbinder.h"

namespace {

QSpinBox *spin(const QString &suffix, int max){
    QSpinBox *spin = new QSpinBox;
    spin->setSuffix(suffix);
    spin->setMaximum(max);
    return spin;
}

}

NotificationsSettings::NotificationsSettings(){
    settings_item = new settings_page("notifications/notifications", "Notifications", "preferences-desktop-notifications");
    settings_item->set_keywords({"popups", "timeout", "alerts"});

    // Popups read these when shown; nothing to reload.
    SettingsBinder *binder = new SettingsBinder("Forest", "Forest", notificationsconfig::group, this);
    connect(settings_item, &settings_category::opened, binder, &SettingsBinder::load);

    settings_widget_group *timeout_group = new settings_widget_group("Timeouts");
    settings_item->add_child(timeout_group);
    QSpinBox *default_timeout = spin(" sec", 3600);
    binder->bind(default_timeout, notificationsconfig::default_timeout, notificationsconfig::default_timeout_default);
    timeout_group->add_child(new settings_widget("Default", "For notifications that don't set their own", default_timeout));
    QSpinBox *min_timeout = spin(" sec", 3600);
    binder->bind(min_timeout, notificationsconfig::min_timeout, notificationsconfig::min_timeout_default);
    timeout_group->add_child(new settings_widget("Minimum", "", min_timeout));
    QSpinBox *max_timeout = spin(" sec", 3600);
    binder->bind(max_timeout, notificationsconfig::max_timeout, notificationsconfig::max_timeout_default);
    timeout_group->add_child(new settings_widget("Maximum", "", max_timeout));

    settings_widget_group *size_group = new settings_widget_group("Size");
    settings_item->add_child(size_group);
    QSpinBox *height = spin("%", 100);
    binder->bind(height, notificationsconfig::height, notificationsconfig::height_default, SettingsBinder::percent());
    size_group->add_child(new settings_widget("Maximum height", "Share of the screen height", height));
    QSpinBox *width = spin("%", 100);
    binder->bind(width, notificationsconfig::width, notificationsconfig::width_default, SettingsBinder::percent());
    size_group->add_child(new settings_widget("Maximum width", "Share of the screen width", width));

    settings_widget_group *test_group = new settings_widget_group;
    settings_item->add_child(test_group);
    QPushButton *test_button = new QPushButton("Test Notification");
    test_group->add_child(new settings_widget("Preview", "", test_button));
    connect(test_button, &QPushButton::clicked, this, &NotificationsSettings::send_test_notification);
}

void NotificationsSettings::send_test_notification(){
    if (QDBusConnection::sessionBus().isConnected()){
        QDBusInterface iface("org.freedesktop.Notifications", "/org/freedesktop/Notifications");
        if (iface.isValid())
            iface.call("Notify", "Forest Settings", uint(0),
                       "preferences-desktop-notifications",
                       "Test Notification", "This is a sample notification.",
                       QStringList(), QVariantMap(), -1);
    }
}
