// SPDX-License-Identifier: LGPL-3.0-or-later

#include "windowlistsettings.h"

#include <QCheckBox>
#include <QSpinBox>

#include "miscutills.h"
#include "settingsbinder.h"
#include "windowlistconfig.h"

QList<settings_page*> WindowListSettings::pages()
{
    settings_page *page = new settings_page(windowlistconfig::path, "Window List", "preferences-system-windows");
    page->set_keywords({"taskbar", "tasks", "windows", "thumbnail", "preview", "button"});

    SettingsBinder *binder = new SettingsBinder("Forest", "Window List", QString(), this);
    binder->set_callback([]{ miscutills::call_dbus("forest/panel/reloadappletsettings", {QString(windowlistconfig::path)}); });
    connect(page, &settings_page::opened, binder, &SettingsBinder::load);

    QCheckBox *previews = new QCheckBox;
    binder->bind(previews, windowlistconfig::showthumbnails, windowlistconfig::showthumbnails_default);
    page->add_child(new settings_widget("Window previews", "Show a thumbnail when hovering a button", previews));

    QSpinBox *max_width = new QSpinBox;
    max_width->setRange(40, 1000);
    max_width->setSuffix(" px");
    binder->bind(max_width, windowlistconfig::maxbuttonsize, windowlistconfig::maxbuttonsize_default);
    page->add_child(new settings_widget("Maximum button width", "", max_width));

    return {page};
}
