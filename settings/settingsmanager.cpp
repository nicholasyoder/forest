// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingsmanager.h"

#include <QDebug>
#include <QDir>
#include <QHoverEvent>
#include <QIconEngine>
#include <QPainter>
#include <QHBoxLayout>
#include <QLabel>
#include <QPluginLoader>
#include <QScrollArea>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QVBoxLayout>

namespace {

// Fusion tints selected icons with the highlight color; the QSS border marks selection.
class UntintedIconEngine : public QIconEngine {
public:
    UntintedIconEngine(const QIcon &icon) : icon(icon) {}
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override {
        icon.paint(painter, rect, Qt::AlignCenter, untinted(mode), state);
    }
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
        return icon.pixmap(size, untinted(mode), state);
    }
    QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale) override {
        return icon.pixmap(size, scale, untinted(mode), state);
    }
    QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
        return icon.actualSize(size, untinted(mode), state);
    }
    QIconEngine *clone() const override { return new UntintedIconEngine(icon); }
private:
    static QIcon::Mode untinted(QIcon::Mode mode){ return mode == QIcon::Selected ? QIcon::Normal : mode; }
    QIcon icon;
};

class UntintedIconDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override {
        QStyledItemDelegate::initStyleOption(option, index);
        if (!option->icon.isNull())
            option->icon = QIcon(new UntintedIconEngine(option->icon));
    }
};

const QString plugin_dir = "/usr/lib/forest/settings";
const int PageIdRole = Qt::UserRole;

struct category_info { QString id, title, icon; };

// Tree order. Plugins only name the ID; unknown IDs land in "other".
const QList<category_info> categories = {
    {"about", "About", "help-about"},
    {"appearance", "Appearance", "preferences-desktop-theme"},
    {"desktop", "Panel", "preferences-desktop"},
    {"displays", "Displays", "preferences-desktop-display"},
    {"input", "Input & Hotkeys", "preferences-desktop-keyboard"},
    {"notifications", "Notifications", "preferences-desktop-notifications"},
    {"power", "Power & Lock", "system-lock-screen"},
    {"session", "Session & Startup", "preferences-system-session"},
    {"system", "System", "preferences-system"},
    {"other", "Other", "preferences-other"},
};

}

SettingsManager::SettingsManager(){
    this->setWindowTitle("Forest Settings");
    this->setWindowIcon(QIcon::fromTheme("preferences-system"));

    setObjectName("SettingsWindow");

    stacked_layout = new QStackedLayout;
    stacked_layout->setContentsMargins(QMargins(0,0,0,0));

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->setContentsMargins(QMargins(0,0,0,0));
    hlayout->setSpacing(0);

    model = new QStandardItemModel(this);
    tree = new QTreeView;
    tree->setObjectName("CategoryTree");
    tree->setModel(model);
    tree->setHeaderHidden(true);
    tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tree->setExpandsOnDoubleClick(false);
    tree->setIconSize(QSize(22, 22));
    tree->setMinimumWidth(200);
    tree->setItemDelegate(new UntintedIconDelegate(tree));
    tree->viewport()->installEventFilter(this);
    connect(tree->selectionModel(), &QItemSelectionModel::currentChanged, this, &SettingsManager::current_changed);
    connect(tree, &QTreeView::clicked, this, &SettingsManager::item_clicked);
    connect(tree, &QTreeView::expanded, this, &SettingsManager::collapse_others);

    hlayout->addWidget(tree);
    hlayout->addLayout(stacked_layout, 1);

    this->resize(850,600);
    QTimer::singleShot(0, this, &SettingsManager::load_settings_ui);
}

SettingsManager::~SettingsManager(){}

bool SettingsManager::eventFilter(QObject *watched, QEvent *event){
    // At fractional scales the hover border bleeds past the row's update rect and leaves lines behind.
    if (watched == tree->viewport() && (event->type() == QEvent::HoverMove || event->type() == QEvent::HoverLeave)) {
        QModelIndex index = event->type() == QEvent::HoverMove
            ? tree->indexAt(static_cast<QHoverEvent*>(event)->position().toPoint()) : QModelIndex();
        if (index != hovered_index) {
            hovered_index = index;
            tree->viewport()->update();
        }
    }
    return QFrame::eventFilter(watched, event);
}

void SettingsManager::load_settings_ui(){
    load_plugins();
    build_tree();
    open_path(initial_path);
}

void SettingsManager::load_plugins(){
    QDir dir(plugin_dir);
    foreach (QString file, dir.entryList({"*.so"}, QDir::Files, QDir::Name)) {
        QPluginLoader plugloader(dir.filePath(file));
        settings_plugin_interface *plugin = qobject_cast<settings_plugin_interface*>(plugloader.instance());
        if (!plugin) {
            qWarning() << "Not a settings plugin:" << file << plugloader.errorString();
            continue;
        }
        foreach (settings_page *page, plugin->pages()) {
            if (!page) continue;
            if (page_hash.contains(page->id())) continue;
            pages.append(page);
            page_hash[page->id()] = page;
        }
    }
}

void SettingsManager::build_tree(){
    // Parents before children, siblings by (order, title).
    std::stable_sort(pages.begin(), pages.end(), [](settings_page *a, settings_page *b){
        int da = a->path().count('/'), db = b->path().count('/');
        if (da != db) return da < db;
        if (a->order() != b->order()) return a->order() < b->order();
        return a->name().localeAwareCompare(b->name()) < 0;
    });

    foreach (const category_info &cat, categories) {
        QStandardItem *item = new QStandardItem(QIcon::fromTheme(cat.icon), cat.title);
        model->appendRow(item);
        path_items[cat.id] = item;
    }

    foreach (settings_page *page, pages) {
        QString path = page->path();
        if (path_items.contains(path)) {
            qWarning() << "Duplicate settings page path:" << path;
            continue;
        }
        QString cat_id = path.section('/', 0, 0);
        QString parent_path = path.section('/', 0, -2);
        QStandardItem *parent = nullptr;
        if (path.count('/') == 0 || !path_items.contains(cat_id)) {
            qWarning() << "Unknown settings category:" << path;
            parent = path_items["other"];
        }
        else if (parent_path == cat_id) {
            parent = path_items[cat_id];
        }
        else {
            parent = path_items.value(parent_path);
            if (!parent) {
                qWarning() << "Missing parent page for" << path;
                parent = path_items[cat_id];
            }
        }

        QStandardItem *item = new QStandardItem(QIcon::fromTheme(page->icon()), page->name());
        item->setData(page->id(), PageIdRole);
        parent->appendRow(item);
        path_items[path] = item;
    }

    // Hide empty categories; a single page with no subpages becomes the category itself.
    for (int row = model->rowCount() - 1; row >= 0; row--) {
        QStandardItem *cat = model->item(row);
        if (!cat->hasChildren()) {
            path_items.remove(path_items.key(cat));
            model->removeRow(row);
        }
        else if (cat->rowCount() == 1 && !cat->child(0)->hasChildren()) {
            QStandardItem *only = cat->child(0);
            cat->setData(only->data(PageIdRole), PageIdRole);
            path_items[path_items.key(only)] = cat;
            cat->removeRow(0);
        }
    }
}

void SettingsManager::open_path(QString path){
    while (path.endsWith('/')) path.chop(1);
    QStandardItem *item = path_items.value(path);
    if (!item && !path.isEmpty())
        qWarning() << "Unknown settings page:" << path;
    if (!item)
        item = model->item(0);
    if (!item) return;
    if (!item->data(PageIdRole).isValid() && item->hasChildren())
        item = item->child(0);
    tree->setCurrentIndex(item->index());
}

void SettingsManager::current_changed(const QModelIndex &index){
    if (!index.isValid()) return;
    QModelIndex top = index;
    while (top.parent().isValid()) top = top.parent();
    collapse_others(top);
    for (QModelIndex p = index.parent(); p.isValid(); p = p.parent())
        tree->expand(p);
    if (model->hasChildren(index))
        tree->expand(index);

    // A multi-page category shows its first page but stays current, so arrow keys can pass it.
    QModelIndex page_index = index;
    while (!page_index.data(PageIdRole).isValid() && model->hasChildren(page_index))
        page_index = model->index(0, 0, page_index);
    if (settings_page *page = page_hash.value(page_index.data(PageIdRole).toUuid()))
        open_page(page);
}

void SettingsManager::item_clicked(const QModelIndex &index){
    if (!index.data(PageIdRole).isValid() && model->hasChildren(index))
        tree->setCurrentIndex(model->index(0, 0, index));
}

void SettingsManager::collapse_others(const QModelIndex &index){
    if (index.parent().isValid()) return;
    for (int row = 0; row < model->rowCount(); row++)
        if (row != index.row())
            tree->collapse(model->index(row, 0));
}

void SettingsManager::open_page(settings_page *page){
    page->notify_opened();
    if (page->child_items().isEmpty()) return;
    // Once only: a second run would rebuild from widgets the first one's page owns.
    connect(page, &settings_category::updated, this, &SettingsManager::update_widgets, Qt::UniqueConnection);
    display_widgets(page->id(), page->child_items());
}

void SettingsManager::update_widgets(QUuid parent_id, QList<settings_item*> items){
    delete stack_hash.take(parent_id);
    display_widgets(parent_id, items);
}

void SettingsManager::display_widgets(QUuid parent_id, QList<settings_item*> items){
    if(stack_hash.contains(parent_id)){
        stacked_layout->setCurrentWidget(stack_hash[parent_id]);
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
                group_v_layout->setContentsMargins(QMargins(0,0,0,0));
                group_v_layout->setSpacing(0);
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
        stack_hash[parent_id] = controls_area;
        stacked_layout->setCurrentWidget(controls_area);
    }
}

QWidget* SettingsManager::create_control(settings_widget* item, QString groupposition){
    QFrame *base_widget = new QFrame;
    QHBoxLayout *base_layout = new QHBoxLayout(base_widget);
    base_layout->setContentsMargins(QMargins(0,0,0,0));
    base_layout->setSpacing(0);
    base_layout->addStretch(0);
    QFrame *control_widget = new QFrame;
    control_widget->setObjectName("ControlWidget");
    control_widget->setProperty("groupposition", groupposition);
    QHBoxLayout *h_layout = new QHBoxLayout(control_widget);
    h_layout->setContentsMargins(QMargins(0,0,0,0));
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
