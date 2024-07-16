#include "settingsmanager.h"

#include <QDebug>
#include <QPluginLoader>
#include <QTimer>

#include "pluginutills.h"

SettingsManager::SettingsManager(){
    this->setWindowTitle("Forest Settings");
    this->setWindowIcon(QIcon::fromTheme("preferences-system"));

    setObjectName("SettingsWindow");

    stacked_layout = new QStackedLayout;
    stacked_layout->setMargin(0);

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->setMargin(0);
    hlayout->setSpacing(0);

    QFrame *left_pane = new QFrame;
    left_pane->setObjectName("CategoriesPane");
    QVBoxLayout *left_v_layout = new QVBoxLayout(left_pane);
    left_v_layout->setMargin(0);
    left_v_layout->setSpacing(0);
    bcw = new breadcrumbwidget;
    connect(bcw, &breadcrumbwidget::level1_activated, this, &SettingsManager::open_home);
    left_v_layout->addWidget(bcw);
    listw = new catlistwidget;
    listw->setObjectName("CategoryList");
    left_v_layout->addWidget(listw);
    hlayout->addWidget(left_pane);
    hlayout->addLayout(stacked_layout, 1);

    listw->setMinimumWidth(175);
    //connect(listw, SIGNAL(currentRowChanged(int)), slayout, SLOT(setCurrentIndex(int)));
    connect(listw, &catlistwidget::currentRowChanged, this, &SettingsManager::open_item);

    this->resize(850,600);

    //loadui();
    QTimer::singleShot(0, this, &SettingsManager::load_settings_ui);
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
            QList<settings_item*> items = pluginterface->get_settings_items();
            load_items(items);
            top_level_items.append(items);
        }
        else {
            qDebug() << plugloader.errorString();
        }
    }
    open_home();
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
        bcw->set_level2_text("");
    else if(item_hash.contains(parent_id))
        bcw->set_level2_text(item_hash[parent_id]->name());
    //    listw->addseperator(item_hash[parent_id]->name());

    if(show_back_button)
        listw->additem(home_id, "Back", QIcon::fromTheme("go-previous"));

    QMap<QString, settings_category*> cat_map;
    foreach(settings_item* item, items){
        settings_category *cat_item = dynamic_cast<settings_category*>(item);
        if(cat_item != nullptr){
            cat_map[cat_item->name()] = cat_item;
        }
    }

    foreach (QString name, cat_map.keys()) {
        settings_category *cat_item = cat_map[name];
        listw->additem(cat_item->id(), cat_item->name(), QIcon::fromTheme(cat_item->icon()));
    }

    // Automaticly open first item - but do second if first item is back button
    //listitem *first_item = (listw->items().first()->text() != "Back") ? listw->items().first() : listw->items().at(1);
    //first_item->activate();

    // Activate first item if were on toplevel
    if (parent_id == home_id)
        listw->items().first()->activate();
}

void SettingsManager::display_widgets(QUuid parent_id, QList<settings_item*> items){
    if(stack_hash.contains(parent_id)){
        stacked_layout->setCurrentIndex(stack_hash[parent_id]);
    }
    else{

        QVBoxLayout *page_layout = new QVBoxLayout();
        foreach(settings_item* item, items){
            settings_widget *widget_item = dynamic_cast<settings_widget*>(item);
            if(widget_item != nullptr){
                QWidget *widget = (widget_item->is_custom()) ? widget_item->widget() : create_control(widget_item);
                page_layout->addWidget(widget);
                continue;
            }

            settings_widget_group *widget_group = dynamic_cast<settings_widget_group*>(item);
            if(widget_group != nullptr){
                QFrame *widget_group_frame = new QFrame;
                widget_group_frame->setObjectName("WidgetGroup");
                QVBoxLayout *group_v_layout = new QVBoxLayout(widget_group_frame);
                group_v_layout->setSpacing(0);
                group_v_layout->setMargin(0);
                foreach (settings_item* sub_item, widget_group->child_items()) {
                    settings_widget *sub_widget_item = dynamic_cast<settings_widget*>(sub_item);
                    if(sub_widget_item != nullptr){
                        QString groupposition = "middle";
                        if (sub_item == widget_group->child_items().first())
                            groupposition = "first";
                        else if (sub_item == widget_group->child_items().last())
                            groupposition = "last";
                        QWidget *widget = (sub_widget_item->is_custom()) ? sub_widget_item->widget() : create_control(sub_widget_item, groupposition);
                        group_v_layout->addWidget(widget);
                        continue;
                    }
                }
                page_layout->addWidget(widget_group_frame);
            }
        }
        page_layout->addStretch(1);


        QFrame *controls_pane = new QFrame;
        controls_pane->setObjectName("ControlsPane");
        controls_pane->setLayout(page_layout);

        QScrollArea *controls_area = new QScrollArea;
        controls_area->setObjectName("ControlsScrollArea");
        controls_area->setWidget(controls_pane);
        controls_area->setWidgetResizable(true);
        controls_area->setFocusPolicy(Qt::NoFocus);


        stacked_layout->addWidget(controls_area);
        int index = stacked_layout->indexOf(controls_area);
        stack_hash[parent_id] = index;
        stacked_layout->setCurrentIndex(index);
    }
}

QWidget* SettingsManager::create_control(settings_widget* item, QString groupposition){
    QFrame *base_widget = new QFrame;
    QHBoxLayout *base_layout = new QHBoxLayout(base_widget);
    base_layout->setMargin(0);
    base_layout->setSpacing(0);
    base_layout->addStretch(0);
    QFrame *control_widget = new QFrame;
    control_widget->setObjectName("ControlWidget");
    control_widget->setProperty("groupposition", groupposition);
    QHBoxLayout *h_layout = new QHBoxLayout(control_widget);
    h_layout->setMargin(0);
    h_layout->setSpacing(0);
    if(item->name() != ""){
        QLabel *name_label = new QLabel(item->name());
        h_layout->addWidget(name_label, 1);
    }
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
