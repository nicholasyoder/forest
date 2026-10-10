// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cpumonitorsettings.h"

#include <QLineEdit>
#include <QSpinBox>

#include "colorbutton.h"
#include "cpumonitorconfig.h"
#include "miscutills.h"
#include "settingsbinder.h"

using namespace cpumonitorconfig;

QList<settings_page*> CpuMonitorSettings::pages()
{
    settings_page *page = new settings_page(path, "CPU Monitor", "utilities-system-monitor");
    page->set_keywords({"processor", "usage", "graph", "load", "color"});

    SettingsBinder *binder = new SettingsBinder("Forest", "CPU Monitor", QString(), this);
    binder->set_callback([]{ miscutills::call_dbus("forest/panel/reloadappletsettings", {QString(path)}); });
    connect(page, &settings_page::opened, binder, &SettingsBinder::load);

    settings_widget_group *colors = new settings_widget_group("Colors");
    ColorButton *graph_color = new ColorButton;
    binder->bind_color(graph_color, foregroundcolor, foregroundopacity, foregroundcolor_default);
    colors->add_child(new settings_widget("Graph", "", graph_color));
    ColorButton *back_color = new ColorButton;
    binder->bind_color(back_color, backgroundcolor, backgroundopacity, backgroundcolor_default);
    colors->add_child(new settings_widget("Background", "", back_color));
    page->add_child(colors);

    settings_widget_group *behavior = new settings_widget_group("Behavior");
    QSpinBox *interval = new QSpinBox;
    interval->setRange(100, 10000);
    interval->setSingleStep(100);
    interval->setSuffix(" ms");
    binder->bind(interval, updateinterval, updateinterval_default);
    behavior->add_child(new settings_widget("Update interval", "", interval));
    QSpinBox *width_box = new QSpinBox;
    width_box->setRange(10, 500);
    width_box->setSuffix(" px");
    binder->bind(width_box, width, width_default);
    behavior->add_child(new settings_widget("Width", "", width_box));
    QLineEdit *command_edit = new QLineEdit;
    command_edit->setPlaceholderText("e.g. lxtask");
    binder->bind(command_edit, command, QString());
    behavior->add_child(new settings_widget("Click command", "Run when the graph is clicked", command_edit));
    page->add_child(behavior);

    return {page};
}
