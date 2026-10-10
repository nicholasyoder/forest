// SPDX-License-Identifier: LGPL-3.0-or-later

#include "clocksettings.h"

#include <QCheckBox>
#include <QComboBox>

#include "clockconfig.h"
#include "miscutills.h"
#include "settingsbinder.h"

QList<settings_page*> ClockSettings::pages()
{
    settings_page *page = new settings_page(clockconfig::path, "Clock", "preferences-system-time");
    page->set_keywords({"time", "format", "12-hour", "24-hour", "am", "pm", "seconds"});

    SettingsBinder *binder = new SettingsBinder("Forest", "Clock", QString(), this);
    binder->set_callback([]{ miscutills::call_dbus("forest/panel/reloadappletsettings", {QString(clockconfig::path)}); });
    connect(page, &settings_page::opened, binder, &SettingsBinder::load);

    QComboBox *format_combo = new QComboBox;
    format_combo->addItem("24-hour", false);
    format_combo->addItem("12-hour", true);
    binder->bind(format_combo, clockconfig::twelvehour, clockconfig::twelvehour_default, SettingsBinder::item_data(format_combo));
    page->add_child(new settings_widget("Time format", "", format_combo));

    QCheckBox *seconds_check = new QCheckBox;
    binder->bind(seconds_check, clockconfig::showseconds, clockconfig::showseconds_default);
    page->add_child(new settings_widget("Show seconds", "", seconds_check));

    return {page};
}
