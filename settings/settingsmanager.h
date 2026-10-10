// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QFrame>
#include <QHash>
#include <QLineEdit>
#include <QPointer>
#include <QStackedLayout>
#include <QStandardItemModel>
#include <QTreeView>

#include "settings_plugin_interface.h"

class SearchFilterModel;

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
    void open_index(QModelIndex index);
    void collapse_others(const QModelIndex &index);
    void update_widgets(QUuid parent_id, QList<settings_item*> items);
    void search_changed(const QString &text);

private:
    void load_plugins();
    void build_tree();
    void set_search_text(QStandardItem *item, settings_page *page);
    void open_page(settings_page *page);
    void highlight_match(const QModelIndex &page_index);
    QModelIndex first_match();
    void display_widgets(QUuid parent_id, QList<settings_item*> items);
    QWidget* create_control(QUuid page_id, settings_widget* item, QString groupposition = "middle");

    QTreeView *tree = nullptr;
    QStandardItemModel *model = nullptr;
    SearchFilterModel *proxy = nullptr;
    QLineEdit *search_field = nullptr;
    QStackedLayout *stacked_layout = nullptr;

    QList<settings_page*> pages;
    QHash<QUuid, settings_page*> page_hash;
    QHash<QString, QStandardItem*> path_items; // category IDs and page paths
    QHash<QUuid, QWidget*> stack_hash; // pages; layout indices shift when one is replaced
    QHash<QUuid, QList<QPair<QString, QPointer<QWidget>>>> row_frames; // page ID -> (label, ControlWidget)
    QPersistentModelIndex shown_index; // source index of the open page
    QString initial_path;
    QPersistentModelIndex hovered_index;
};
#endif // SETTINGSMANAGER_H
