// SPDX-License-Identifier: LGPL-3.0-or-later

#include "notificationssettings.h"
#include <QSettings>
#include <QDBusConnection>
#include <QDBusInterface>

#include "../../library/miscutills/miscutills.h"

NotificationsSettings::NotificationsSettings(){
    settings_item = new settings_category("Notifications", "", "preferences-desktop-notifications");
    connect(settings_item, &settings_category::opened, this, &NotificationsSettings::load_settings);

    settings_widget_group *timeout_settings_group = new settings_widget_group;
    settings_item->add_child(timeout_settings_group);

    min_timeout_spinbox = new QSpinBox;
    min_timeout_spinbox->setSuffix(" sec");
    min_timeout_spinbox->setMaximum(3600);
    settings_widget *min_timeout_item = new settings_widget("Min Timeout", "", min_timeout_spinbox);
    timeout_settings_group->add_child(min_timeout_item);

    max_timeout_spinbox = new QSpinBox;
    max_timeout_spinbox->setSuffix(" sec");
    max_timeout_spinbox->setMaximum(3600);
    settings_widget *max_timeout_item = new settings_widget("Max Timeout", "", max_timeout_spinbox);
    timeout_settings_group->add_child(max_timeout_item);

    default_timeout_spinbox = new QSpinBox;
    default_timeout_spinbox->setSuffix(" sec");
    default_timeout_spinbox->setMaximum(3600);
    settings_widget *default_timeout_item = new settings_widget("Default Timeout", "", default_timeout_spinbox);
    timeout_settings_group->add_child(default_timeout_item);

    settings_widget_group *size_settings_group = new settings_widget_group;
    settings_item->add_child(size_settings_group);

    height_spinbox = new QSpinBox;
    height_spinbox->setSuffix("%");
    height_spinbox->setMaximum(100);
    settings_widget *height_item = new settings_widget("Max Height", "", height_spinbox);
    size_settings_group->add_child(height_item);

    width_spinbox = new QSpinBox;
    width_spinbox->setSuffix("%");
    width_spinbox->setMaximum(100);
    settings_widget *width_item = new settings_widget("Max Width", "", width_spinbox);
    size_settings_group->add_child(width_item);

    settings_widget_group *test_group = new settings_widget_group;
    settings_item->add_child(test_group);

    QPushButton *test_button = new QPushButton("Test Notification");
    settings_widget *test_item = new settings_widget("Preview", "", test_button);
    test_group->add_child(test_item);
    connect(test_button, &QPushButton::clicked, this, &NotificationsSettings::send_test_notification);

    RunOnce *runner = new RunOnce(1000);
    connect(runner, &RunOnce::activated, this, &NotificationsSettings::save_settings);
    connect(min_timeout_spinbox, QOverload<int>::of(&QSpinBox::valueChanged), runner, &RunOnce::try_activate);
    connect(max_timeout_spinbox, QOverload<int>::of(&QSpinBox::valueChanged), runner, &RunOnce::try_activate);
    connect(default_timeout_spinbox, QOverload<int>::of(&QSpinBox::valueChanged), runner, &RunOnce::try_activate);
    connect(height_spinbox, QOverload<int>::of(&QSpinBox::valueChanged), runner, &RunOnce::try_activate);
    connect(width_spinbox, QOverload<int>::of(&QSpinBox::valueChanged), runner, &RunOnce::try_activate);
}


void NotificationsSettings::load_settings(){
    QSettings settings("Forest", "Forest");
    settings.beginGroup("notifications");
    min_timeout_spinbox->setValue(settings.value("min_timeout", 3).toInt());
    max_timeout_spinbox->setValue(settings.value("max_timeout", 30).toInt());
    default_timeout_spinbox->setValue(settings.value("default_timeout", 8).toInt());
    height_spinbox->setValue(settings.value("height", 0.7).toReal() * 100);
    width_spinbox->setValue(settings.value("width", 0.5).toReal() * 100);
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

void NotificationsSettings::save_settings(){
    QSettings settings("Forest", "Forest");
    settings.beginGroup("notifications");
    settings.setValue("min_timeout", min_timeout_spinbox->value());
    settings.setValue("max_timeout", max_timeout_spinbox->value());
    settings.setValue("default_timeout", default_timeout_spinbox->value());
    settings.setValue("height", qreal(height_spinbox->value()) / 100);
    settings.setValue("width", qreal(width_spinbox->value()) / 100);
    settings.sync();
}
