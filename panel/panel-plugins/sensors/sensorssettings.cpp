// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sensorssettings.h"

#include <QComboBox>
#include <QSpinBox>
#include <QTimer>

#include "colorbutton.h"
#include "listwidget.h"
#include "miscutills.h"
#include "sensor/sensors.h"
#include "sensorsconfig.h"
#include "settingsbinder.h"

using namespace sensorsconfig;

SensorsSettings::~SensorsSettings(){ delete sensors; }

QList<settings_page*> SensorsSettings::pages()
{
    settings_page *page = new settings_page(path, "Sensor Monitor", "temperature-normal");
    page->set_keywords({"temperature", "thermal", "cpu", "gpu", "lm-sensors", "celsius", "fahrenheit", "bars"});

    QList<QPair<QString, QString>> temp_sensors; // id, shown name
    sensors = new Sensors;
    for (const Chip &chip : sensors->getDetectedChips())
        for (const Feature &feature : chip.getFeatures())
            if (feature.getType() == SENSORS_FEATURE_TEMP)
                temp_sensors.append({sensor_id(chip, feature),
                                     QString::fromStdString(feature.getLabel() + " (" + chip.getName() + ")")});

    SettingsBinder *binder = new SettingsBinder("Forest", "Temperature Monitor", QString(), this);
    binder->set_callback([]{ miscutills::call_dbus("forest/panel/reloadappletsettings", {QString(path)}); });

    settings_widget_group *general = new settings_widget_group("General");
    QComboBox *display_combo = new QComboBox;
    display_combo->addItem("Temperature", display_text);
    display_combo->addItem("Bars", display_bars);
    binder->bind(display_combo, display, display_text, SettingsBinder::item_data(display_combo));
    general->add_child(new settings_widget("Show", "", display_combo));
    QComboBox *scale_combo = new QComboBox;
    scale_combo->addItem("Celsius", false);
    scale_combo->addItem("Fahrenheit", true);
    binder->bind(scale_combo, fahrenheit, fahrenheit_default, SettingsBinder::item_data(scale_combo));
    general->add_child(new settings_widget("Scale", "", scale_combo));
    QSpinBox *interval = new QSpinBox;
    interval->setRange(1, 60);
    interval->setSuffix(" s");
    binder->bind(interval, updateinterval, updateinterval_default);
    general->add_child(new settings_widget("Update interval", "", interval));
    page->add_child(general);

    // Thresholds compare against Celsius readings.
    auto celsius_box = [](int min, int max){
        QSpinBox *box = new QSpinBox;
        box->setRange(min, max);
        box->setSuffix(" °C");
        return box;
    };

    settings_widget_group *text_group = new settings_widget_group("Temperature");
    QComboBox *sensor_combo = new QComboBox;
    for (const auto &temp_sensor : temp_sensors)
        sensor_combo->addItem(temp_sensor.second, temp_sensor.first);
    binder->bind(sensor_combo, sensor, temp_sensors.isEmpty() ? QString() : temp_sensors.first().first,
                 SettingsBinder::item_data(sensor_combo));
    text_group->add_child(new settings_widget("Sensor", "", sensor_combo));
    QSpinBox *warning = celsius_box(0, 200);
    binder->bind(warning, warningtemp, warningtemp_default);
    text_group->add_child(new settings_widget("Warning at", "Shown yellow from here", warning));
    QSpinBox *critical = celsius_box(0, 200);
    binder->bind(critical, criticaltemp, criticaltemp_default);
    text_group->add_child(new settings_widget("Critical at", "Shown red from here", critical));
    page->add_child(text_group);

    settings_widget_group *bars_group = new settings_widget_group("Bars");
    ListWidget *bar_list = new ListWidget;
    // Filled on open: ListWidget sizes itself correctly only while visible.
    connect(page, &settings_page::opened, bar_list, [bar_list, temp_sensors]{
        QSignalBlocker blocker(bar_list);
        bar_list->clear();
        for (const auto &temp_sensor : temp_sensors) {
            QListWidgetItem *item = new QListWidgetItem(temp_sensor.second);
            item->setData(Qt::UserRole, temp_sensor.first);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
            bar_list->addItem(item);
        }
    });
    binder->bind_custom(bar_list, &QListWidget::itemChanged,
        [bar_list](QSettings &settings){
            QStringList hidden = settings.value(hiddenbars).toStringList();
            for (int i = 0; i < bar_list->count(); i++) {
                QListWidgetItem *item = bar_list->item(i);
                item->setCheckState(hidden.contains(item->data(Qt::UserRole).toString()) ? Qt::Unchecked : Qt::Checked);
            }
        },
        [bar_list](QSettings &settings){
            QStringList hidden;
            for (int i = 0; i < bar_list->count(); i++)
                if (bar_list->item(i)->checkState() == Qt::Unchecked)
                    hidden.append(bar_list->item(i)->data(Qt::UserRole).toString());
            settings.setValue(hiddenbars, hidden);
        });
    bars_group->add_child(new settings_widget("", "", bar_list));
    QSpinBox *max = celsius_box(1, 200);
    binder->bind(max, maxtemp, maxtemp_default);
    bars_group->add_child(new settings_widget("Full bar at", "", max));
    QSpinBox *width_box = new QSpinBox;
    width_box->setRange(1, 50);
    width_box->setSuffix(" px");
    binder->bind(width_box, barwidth, barwidth_default);
    bars_group->add_child(new settings_widget("Bar width", "", width_box));
    QSpinBox *spacing_box = new QSpinBox;
    spacing_box->setRange(0, 50);
    spacing_box->setSuffix(" px");
    binder->bind(spacing_box, barspacing, barspacing_default);
    bars_group->add_child(new settings_widget("Bar spacing", "", spacing_box));
    QSpinBox *margin_box = new QSpinBox;
    margin_box->setRange(0, 50);
    margin_box->setSuffix(" px");
    binder->bind(margin_box, margin, margin_default);
    bars_group->add_child(new settings_widget("Margin", "", margin_box));
    ColorButton *back_color = new ColorButton;
    binder->bind(back_color, backcolor, backcolor_default);
    bars_group->add_child(new settings_widget("Background", "", back_color));
    page->add_child(bars_group);

    // After the list fill above.
    connect(page, &settings_page::opened, binder, &SettingsBinder::load);

    QList<QWidget*> text_rows = {sensor_combo, warning, critical};
    QList<QWidget*> bar_rows = {bar_list, max, width_box, spacing_box, margin_box, back_color};
    // Deferred: on first open, opened (and the load) runs before the rows exist, and showing
    // a parentless control opens it as a window.
    auto sync_mode = [display_combo, text_rows, bar_rows]{
        QTimer::singleShot(0, display_combo, [display_combo, text_rows, bar_rows]{
            bool bars = display_combo->currentData().toString() == display_bars;
            foreach (QWidget *row, text_rows) row->setVisible(!bars);
            foreach (QWidget *row, bar_rows) row->setVisible(bars);
        });
    };
    connect(display_combo, &QComboBox::currentIndexChanged, page, sync_mode);
    connect(page, &settings_page::opened, page, sync_mode);

    return {page};
}
