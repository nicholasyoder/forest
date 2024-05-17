#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QMainWindow>
#include <QSettings>
#include <QHBoxLayout>
#include <QHash>

#include "listwidget.h"
#include "settings_plugin_interface.h"

class SettingsManager : public QFrame
{
    Q_OBJECT

public:
    SettingsManager();
    ~SettingsManager();

    void load_settings_ui();
    void load_items(QList<settings_item*> items);
    void display_categories(QList<settings_item*> items, bool show_back_button = false);

private slots:
    void open_item(QUuid id);

private:
    listwidget *listw = nullptr;
    QHBoxLayout *widget_layout = nullptr;
    QHash<QUuid, settings_item*> item_hash;
    QList<settings_item*> top_level_items;
    QUuid home_id = QUuid::createUuid();
};
#endif // SETTINGSMANAGER_H
