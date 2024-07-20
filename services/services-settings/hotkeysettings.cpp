#include "hotkeysettings.h"

#include <QSettings>

HotkeySettings::HotkeySettings(){
    settings_item = new settings_category("Hotkeys", "", "preferences-desktop-keyboard");
    connect(settings_item, &settings_category::opened, this, &HotkeySettings::load_hotkeys);

    settings_widget_group *hotkey_wg = new settings_widget_group;
    settings_item->add_child(hotkey_wg);

    QSettings settings("Forest", "Forest");
    settings.beginGroup("hotkeys");
    foreach(QString key, settings.childGroups()){
        settings.beginGroup(key);

        QWidget *item_widget = new QWidget;
        settings_widget *desktop_theme_list_item = new settings_widget(settings.value("description").toString(),"", item_widget);
        hotkey_wg->add_child(desktop_theme_list_item);

        settings.endGroup();
    }

}

void HotkeySettings::load_hotkeys(){

}
