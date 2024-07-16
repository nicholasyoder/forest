/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2024 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "panelsettings.h"

#include <QSettings>
#include <QPluginLoader>
#include <QTimer>

#include "../panel-library/panelpluginterface.h"

PanelSettings::PanelSettings()
{

}

QList<settings_item*> PanelSettings::get_settings_items(){
    QList<settings_item*> items;

    settings_category *panel_cat = new settings_category("Panel", "", "preferences-desktop");
    items.append(panel_cat);

    settings_category *behavior_cat = new settings_category("Behavior", "", "configure");
    connect(behavior_cat, &settings_category::opened, this, &PanelSettings::load_behavior_settings);
    panel_cat->add_child(behavior_cat);

    position_select = new QComboBox();
    position_select->addItem("Top");
    position_select->addItem("Bottom");
    settings_widget *position_item = new settings_widget("Position", "", position_select);
    behavior_cat->add_child(position_item);


    settings_category *applets_cat = new settings_category("Applets", "", "preferences-plugin");
    connect(applets_cat, &settings_category::opened, this, &PanelSettings::load_applets);
    panel_cat->add_child(applets_cat);

    applet_list_w = new ListWidget;
    applet_list_w->setDragDropMode(QAbstractItemView::InternalMove);
    connect(applet_list_w, &QListWidget::itemChanged, this, &PanelSettings::set_applets);
    ReorderListener *rl = new ReorderListener(applet_list_w);
    connect(rl, &ReorderListener::reordered, this, &PanelSettings::set_applets);
    applet_list_w->installEventFilter(rl);
    settings_widget *applet_list_item = new settings_widget("", "", applet_list_w);
    applets_cat->add_child(applet_list_item);

    return items;
}

void PanelSettings::load_behavior_settings(){
    QSettings settings("Forest", "Panel");

    QString position = settings.value("position").toString();
    position_select->setCurrentText(position);

    connect(position_select, &QComboBox::currentTextChanged, this, &PanelSettings::set_behavior_settings);
}

void PanelSettings::set_behavior_settings(){
    QSettings settings("Forest", "Panel");
    settings.setValue("position", position_select->currentText());
    settings.sync();

    miscutills::call_dbus("forest/panel/reloadsettings");
}

void PanelSettings::load_applets(){
    applet_list_w->clear();

    QSettings settings("Forest", "Panel");
    settings.beginGroup("plugins");

    foreach(QString key, settings.childGroups()){
        QString path = settings.value(key+"/path").toString();
        bool enabled = settings.value(key+"/enabled", false).toBool();
        if (path == "seperator"){
            QListWidgetItem *item = new QListWidgetItem("Seperator");
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
            applet_list_w->addItem(item);
            path_hash["Seperator"] = path;
        }
        else {
            QPluginLoader plugloader(path);
            if (plugloader.load()){
                QObject *plugin = plugloader.instance();
                if (plugin){
                    if (panelpluginterface *pluginterface = qobject_cast<panelpluginterface *>(plugin)){
                        QString name = pluginterface->getpluginfo()["name"];
                        QListWidgetItem *item = new QListWidgetItem(name);
                        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                        item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
                        applet_list_w->addItem(item);
                        path_hash[name] = path;
                        delete pluginterface;
                    }
                }
            }
            plugloader.unload();
        }
    }
}

void PanelSettings::set_applets(){
    QSettings settings("Forest", "Panel");
    settings.beginGroup("plugins");
    settings.remove("");

    for(int i = 0; i < applet_list_w->count(); i++){
        settings.beginGroup("plug-"+padwithzeros(i));
        settings.setValue("path", path_hash[applet_list_w->item(i)->text()]);
        if (applet_list_w->item(i)->checkState() == Qt::Checked)
            settings.setValue("enabled", true);
        else
            settings.setValue("enabled", false);
        settings.endGroup();
    }
    settings.sync();

    miscutills::call_dbus("forest/panel/reloadplugins");
}

QString PanelSettings::padwithzeros(int number){
    if (number < 10) return "000" + QString::number(number);
    else if (number < 100) return "00" + QString::number(number);
    else if (number < 1000) return "0" + QString::number(number);
    else return QString::number(number);
}
