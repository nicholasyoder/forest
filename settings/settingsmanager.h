// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QFrame>
#include <QHash>
#include <QStackedLayout>
#include <QStandardItemModel>
#include <QTreeView>

#include "settings_plugin_interface.h"

class SettingsManager : public QFrame
{
    Q_OBJECT

public:
    SettingsManager();
    ~SettingsManager();

    void set_initial_path(QString path){ initial_path = path; }

public slots:
    void load_settings_ui();
    void open_path(QString path);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void current_changed(const QModelIndex &index);
    void item_clicked(const QModelIndex &index);
    void collapse_others(const QModelIndex &index);
    void update_widgets(QUuid parent_id, QList<settings_item*> items);

private:
    void load_plugins();
    void build_tree();
    void open_page(settings_page *page);
    void display_widgets(QUuid parent_id, QList<settings_item*> items);
    QWidget* create_control(settings_widget* item, QString groupposition = "middle");

    QTreeView *tree = nullptr;
    QStandardItemModel *model = nullptr;
    QStackedLayout *stacked_layout = nullptr;

    QList<settings_page*> pages;
    QHash<QUuid, settings_page*> page_hash;
    QHash<QString, QStandardItem*> path_items; // category IDs and page paths
    QHash<QUuid, QWidget*> stack_hash; // pages; layout indices shift when one is replaced
    QString initial_path;
    QPersistentModelIndex hovered_index;
};
#endif // SETTINGSMANAGER_H
