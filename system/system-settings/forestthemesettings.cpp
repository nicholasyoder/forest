#include "forestthemesettings.h"

#include <QDir>
#include <QSettings>

#include "miscutills.h"

ForestThemeSettings::ForestThemeSettings(){
    settings_item = new settings_category("Desktop", "", "preferences-desktop-color");
    connect(settings_item, &settings_category::opened, this, &ForestThemeSettings::load_desktop_themes);
    forest_theme_list = new ListWidget();
    forest_theme_list->setMinimumHeight(5);
    forest_theme_list->setExclusiveCheck(true);
    forest_theme_list->setSelectionMode(QAbstractItemView::NoSelection);
    settings_widget *desktop_theme_list_item = new settings_widget("","", forest_theme_list);
    settings_item->add_child(desktop_theme_list_item);

    connect(forest_theme_list, &ListWidget::itemExclusivlySelected, this, &ForestThemeSettings::set_desktop_theme);
}

void ForestThemeSettings::load_desktop_themes(){
    forest_theme_list->clear();

    QSettings settings("Forest", "Forest");
    QString current_theme = settings.value("theme").toString();

    QDir theme_dir("/usr/share/forest/themes");
    QStringList entrylist = theme_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (QString entry, entrylist) {
        QListWidgetItem *item = new QListWidgetItem(entry);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(entry == current_theme ? Qt::Checked : Qt::Unchecked);
        forest_theme_list->addItem(item);
    }
}

void ForestThemeSettings::set_desktop_theme(QListWidgetItem *theme_item){
    QString new_theme = theme_item->text();
    QSettings settings("Forest", "Forest");
    settings.setValue("theme", new_theme);
    settings.sync();
    miscutills::call_dbus("forest/loadstylesheet");
}
