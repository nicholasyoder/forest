#include "settingsmanager.h"

#include <QDebug>
#include <QPluginLoader>

#include "pluginutills.h"

SettingsManager::SettingsManager(){
    this->setWindowTitle("Forest Settings");
    this->setWindowIcon(QIcon::fromTheme("preferences-system"));

    setObjectName("SettingsWindow");

    stacked_layout = new QStackedLayout;
    stacked_layout->setMargin(0);
    listw = new listwidget;

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->setMargin(0);
    hlayout->setSpacing(0);
    hlayout->addWidget(listw);

    QFrame *controls_pane = new QFrame;
    controls_pane->setObjectName("ControlsPane");
    controls_pane->setLayout(stacked_layout);
    hlayout->addWidget(controls_pane, 1);

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

void SettingsManager::display_categories(QUuid parent_id, QList<settings_item*> items, bool show_back_button){
    listw->clear();
    if(parent_id == home_id)
        listw->addseperator("Forest Settings");
    else if(item_hash.contains(parent_id))
        listw->addseperator(item_hash[parent_id]->name());
    if(show_back_button)
        listw->additem(home_id, "Back", QIcon::fromTheme("go-previous"));
    foreach(settings_item* item, items){
        settings_category *cat_item = dynamic_cast<settings_category*>(item);
        if(cat_item != nullptr){
            listw->additem(cat_item->id(), cat_item->name(), QIcon::fromTheme(cat_item->icon()));
        }
    }
}

void SettingsManager::display_widgets(QUuid parent_id, QList<settings_item*> items){
    if(stack_hash.contains(parent_id)){
        stacked_layout->setCurrentIndex(stack_hash[parent_id]);
    }
    else{
        QWidget *page_widget = new QWidget;
        QVBoxLayout *page_layout = new QVBoxLayout(page_widget);
        foreach(settings_item* item, items){
            settings_widget *widget_item = dynamic_cast<settings_widget*>(item);
            if(widget_item != nullptr){
                QWidget *widget = (widget_item->is_custom()) ? widget_item->widget() : create_control(widget_item);
                page_layout->addWidget(widget);
            }
        }
        page_layout->addStretch(1);
        stacked_layout->addWidget(page_widget);
        int index = stacked_layout->indexOf(page_widget);
        stack_hash[parent_id] = index;
        stacked_layout->setCurrentIndex(index);
    }
}

QWidget* SettingsManager::create_control(settings_widget* item){
    QFrame *base_widget = new QFrame;
    QHBoxLayout *base_layout = new QHBoxLayout(base_widget);
    base_layout->setMargin(0);
    base_layout->setSpacing(0);
    base_layout->addStretch(0);
    QFrame *control_widget = new QFrame;
    control_widget->setObjectName("ControlWidget");
    QHBoxLayout *h_layout = new QHBoxLayout(control_widget);
    QLabel *name_label = new QLabel(item->name());
    h_layout->addWidget(name_label, 1);
    h_layout->addWidget(item->widget());
    base_layout->addWidget(control_widget, 1);
    base_layout->addStretch(0);
    return base_widget;
}

void SettingsManager::open_item(QUuid id){
    if(id == home_id){
        display_categories(home_id, top_level_items);
    }
    else if(item_hash.contains(id)){
        settings_category *cat_item = dynamic_cast<settings_category*>(item_hash[id]);
        if(cat_item){
            cat_item->notify_opened();
            if(cat_item->child_items().isEmpty()) return;
            QList<settings_item*> child_items = cat_item->child_items();
            if (dynamic_cast<settings_category*>(child_items.first())){
                // Handle item containing sub categories
                display_categories(cat_item->id(), child_items, true);
            }
            else {
                // Handle other item
                display_widgets(cat_item->id(), child_items);
            }
        }
    }
}
