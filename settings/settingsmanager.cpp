#include "settingsmanager.h"

#include <QDebug>
#include <QPluginLoader>

#include "pluginutills.h"

SettingsManager::SettingsManager(){
    this->setWindowTitle("Forest Settings");
    this->setWindowIcon(QIcon::fromTheme("preferences-system"));

    setObjectName("settingsMainWindow");

    widget_layout = new QHBoxLayout;
    listw = new listwidget;

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->setMargin(0);
    hlayout->setSpacing(0);
    hlayout->addWidget(listw);
    hlayout->addLayout(widget_layout, 1);

    listw->setMinimumWidth(175);
    //connect(listw, SIGNAL(currentRowChanged(int)), slayout, SLOT(setCurrentIndex(int)));
    connect(listw, &listwidget::currentRowChanged, this, &SettingsManager::open_item);

    this->resize(850,600);

    //loadui();
}

SettingsManager::~SettingsManager()
{
}

void SettingsManager::load_settings_ui(){
    QStringList plugin_paths = pluginutills::get_plugin_paths(SETTINGS_PLUGIN);
    foreach (QString plugin_path, plugin_paths) {

        QPluginLoader plugloader(plugin_path);
        if (plugloader.load()) {
            QObject *plugin = plugloader.instance();
            if (!plugin) continue;

            settings_plugin_infterace *pluginterface = qobject_cast<settings_plugin_infterace *>(plugin);
            if (!pluginterface) continue;

            //QString plugin_name = pluginterface->get_name();
            //QString plugin_icon = pluginterface->get_icon();
            QList<settings_item*> items = pluginterface->get_settings_items();
            //interface_hash[plugin_name] = pluginterface;
            //listw->additem(plugin_name, QIcon::fromTheme(plugin_icon));

            // load items into hash
            load_items(items);
            top_level_items.append(items);
        }
        else {
            qDebug() << plugloader.errorString();
        }
    }
    open_item(home_id);
}

void SettingsManager::load_items(QList<settings_item*> items){
    foreach(settings_item* item, items){
        // TODO: add name and category to search hash
        item_hash[item->id()] = item;

        settings_category *cat_item = dynamic_cast<settings_category*>(item);
        if(cat_item != nullptr){

            load_items(cat_item->child_items());
        }
    }
}

void SettingsManager::display_categories(QList<settings_item*> items, bool show_back_button){
    listw->clear();
    if(show_back_button){
        listw->additem(home_id, "Back", QIcon::fromTheme("go-previous"));

    }
    foreach(settings_item* item, items){
        settings_category *cat_item = dynamic_cast<settings_category*>(item);
        if(cat_item != nullptr){
            listw->additem(cat_item->id(), cat_item->name(), QIcon::fromTheme(cat_item->icon()));
        }
    }
}

void SettingsManager::open_item(QUuid id){
    if(id == home_id){
        display_categories(top_level_items);
    }
    else if(item_hash.contains(id)){
        settings_category *cat_item = dynamic_cast<settings_category*>(item_hash[id]);
        if(cat_item){
            if(cat_item->child_items().isEmpty()) return;
            QList<settings_item*> child_items = cat_item->child_items();
            if (dynamic_cast<settings_category*>(child_items.first())){
                // Handle item containing sub categories
                display_categories(child_items, true);
            }
            else {
                // Handle other item
                qDebug() << 4;
            }
        }
    }
}
