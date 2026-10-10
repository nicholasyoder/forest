// SPDX-License-Identifier: LGPL-3.0-or-later

#include "panelsettings.h"

#include <QCheckBox>
#include <QSettings>
#include <QPluginLoader>
#include <QTimer>

#include "../panel-library/panelpluginterface.h"
#include "panelconfig.h"
#include "settingsbinder.h"
#include "settingsrow.h"

PanelSettings::PanelSettings()
{

}

QList<settings_page*> PanelSettings::pages(){
    settings_page *panel_page = new settings_page("desktop/panel", "Panel", "preferences-desktop");
    panel_page->set_keywords({"taskbar", "applets", "plugins", "widgets", "position", "autohide", "top", "bottom"});

    SettingsBinder *binder = new SettingsBinder("Forest", "Panel", QString(), this);
    binder->set_callback([]{ miscutills::call_dbus("forest/panel/reloadsettings"); });
    connect(panel_page, &settings_category::opened, binder, &SettingsBinder::load);
    connect(panel_page, &settings_category::opened, this, &PanelSettings::load_applets);

    settings_widget_group *behavior_group = new settings_widget_group("Behavior");
    panel_page->add_child(behavior_group);

    QComboBox *position_select = new QComboBox();
    position_select->addItems({"Top", "Bottom"});
    binder->bind(position_select, panelconfig::position, panelconfig::position_default);
    behavior_group->add_child(new settings_widget("Position", "", position_select));

    QCheckBox *autohide_check = new QCheckBox();
    binder->bind(autohide_check, panelconfig::autohide, panelconfig::autohide_default);
    behavior_group->add_child(new settings_widget("Hide when not in use", "", autohide_check));

    QSpinBox *autohide_delay_input = new QSpinBox();
    autohide_delay_input->setRange(0, 10000);
    autohide_delay_input->setSingleStep(100);
    autohide_delay_input->setSuffix(" ms");
    binder->bind(autohide_delay_input, panelconfig::autohide_delay, panelconfig::autohide_delay_default);
    behavior_group->add_child(new settings_widget("Hide delay", "", autohide_delay_input));
    connect(autohide_check, &QCheckBox::toggled, autohide_delay_input, &QWidget::setVisible);
    autohide_delay_input->setVisible(autohide_check->isChecked());

    settings_widget_group *applets_group = new settings_widget_group("Applets");
    panel_page->add_child(applets_group);
    applet_list_w = new ListWidget;
    applet_list_w->setDragDropMode(QAbstractItemView::InternalMove);
    connect(applet_list_w, &QListWidget::itemChanged, this, &PanelSettings::set_applets);
    ReorderListener *rl = new ReorderListener(applet_list_w);
    connect(rl, &ReorderListener::reordered, this, &PanelSettings::set_applets);
    applet_list_w->installEventFilter(rl);
    applets_group->add_child(new settings_widget("", "", applet_list_w));

    return {panel_page};
}

void PanelSettings::load_applets(){
    applet_list_w->clear();

    QSettings settings("Forest", "Panel");
    settings.beginGroup("plugins");

    foreach(QString key, settings.childGroups()){
        QString path = settings.value(key+"/path").toString();
        bool enabled = settings.value(key+"/enabled", false).toBool();
        if (path == "separator"){
            QListWidgetItem *item = new QListWidgetItem("Separator");
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
            applet_list_w->addItem(item);
            path_hash["Separator"] = path;
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
    bool ok = settings.status() == QSettings::NoError;
    settingsrow::flash(settingsrow::row_of(applet_list_w), "saved", ok ? "true" : "error", ok ? 1000 : 3000);

    miscutills::call_dbus("forest/panel/reloadplugins");
}

QString PanelSettings::padwithzeros(int number){
    if (number < 10) return "000" + QString::number(number);
    else if (number < 100) return "00" + QString::number(number);
    else if (number < 1000) return "0" + QString::number(number);
    else return QString::number(number);
}
