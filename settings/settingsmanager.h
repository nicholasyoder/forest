#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QMainWindow>
#include <QSettings>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHash>
#include <QMap>
#include <QStackedLayout>
#include <QScrollArea>


#include "breadcrumbwidget.h"
#include "catlistwidget.h"
#include "settings_plugin_interface.h"

class SettingsManager : public QFrame
{
    Q_OBJECT

public:
    SettingsManager();
    ~SettingsManager();

    void load_items(QList<settings_item*> items);
    void display_categories(QUuid parent_id, QList<settings_item*> items, bool show_back_button = false);
    void display_widgets(QUuid parent_id, QList<settings_item*> items);
    void update_widgets(QUuid parent_id, QList<settings_item*> items);
    QWidget* create_control(settings_widget* item, QString groupposition = "middle");

public slots:
    void load_settings_ui();

private slots:
    void open_item(QUuid id);
    void open_home(){ open_item(home_id); }

private:
    breadcrumbwidget *bcw = nullptr;
    catlistwidget *listw = nullptr;
    QStackedLayout *stacked_layout = nullptr;

    QHash<QUuid, settings_item*> item_hash;
    QList<settings_item*> top_level_items;
    QUuid home_id = QUuid::createUuid();
    QHash<QUuid, int> stack_hash;
};
#endif // SETTINGSMANAGER_H
