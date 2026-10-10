// SPDX-License-Identifier: LGPL-3.0-or-later

#include "memorymonitorsettings.h"

#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTimer>

#include "colorbutton.h"
#include "memorymonitorconfig.h"
#include "miscutills.h"
#include "settingsbinder.h"

using namespace memorymonitorconfig;

QList<settings_page*> MemoryMonitorSettings::pages()
{
    settings_page *page = new settings_page(path, "Memory Monitor", "utilities-system-monitor");
    page->set_keywords({"ram", "swap", "usage", "graph", "color"});

    SettingsBinder *binder = new SettingsBinder("Forest", "Memory Monitor", QString(), this);
    binder->set_callback([]{ miscutills::call_dbus("forest/panel/reloadappletsettings", {QString(path)}); });
    connect(page, &settings_page::opened, binder, &SettingsBinder::load);

    settings_widget_group *swap = new settings_widget_group("Swap");
    QComboBox *swap_combo = new QComboBox;
    swap_combo->addItem("Ignore", swap_disabled);
    swap_combo->addItem("Combine with RAM", swap_combine);
    swap_combo->addItem("Show separately", swap_separate);
    binder->bind(swap_combo, swapbehavior, swap_combine, SettingsBinder::item_data(swap_combo));
    swap->add_child(new settings_widget("Swap usage", "", swap_combo));

    settings_widget_group *colors = new settings_widget_group("Colors");
    ColorButton *ram_color = new ColorButton;
    binder->bind_color(ram_color, ramcolor, ramopacity, ramcolor_default);
    colors->add_child(new settings_widget("RAM", "", ram_color));
    ColorButton *swap_color = new ColorButton;
    binder->bind_color(swap_color, swapcolor, swapopacity, swapcolor_default);
    colors->add_child(new settings_widget("Swap", "", swap_color));
    ColorButton *back_color = new ColorButton;
    binder->bind_color(back_color, backgroundcolor, backgroundopacity, backgroundcolor_default);
    colors->add_child(new settings_widget("Background", "", back_color));
    // Only a separate swap graph uses its own color.
    // Deferred: on first open, opened (and the load) runs before the rows exist, and showing
    // a parentless control opens it as a window.
    auto sync_swap_color = [swap_combo, swap_color]{
        QTimer::singleShot(0, swap_color, [swap_combo, swap_color]{
            swap_color->setVisible(swap_combo->currentData().toString() == swap_separate);
        });
    };
    connect(swap_combo, &QComboBox::currentIndexChanged, swap_color, sync_swap_color);
    connect(page, &settings_page::opened, swap_color, sync_swap_color);

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

    page->add_child(swap);
    page->add_child(colors);
    page->add_child(behavior);
    return {page};
}
