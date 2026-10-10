// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingsmanager.h"
#include "settingsrow.h"
#include "xdgactivation.h"

#include <QDebug>
#include <QDir>
#include <QHoverEvent>
#include <QIconEngine>
#include <QPainter>
#include <QHBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QPluginLoader>
#include <QScrollArea>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QStyle>
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
const int SearchTextRole = Qt::UserRole + 1; // title, category, keywords, row labels
const int BaseSearchTextRole = Qt::UserRole + 2; // without row labels

QStringList query_words(const QString &query){
    return query.toLower().split(' ', Qt::SkipEmptyParts);
}

QStringList row_labels(const QList<settings_item*> &items){
    QStringList labels;
    foreach (settings_item *item, items) {
        if (!item->name().isEmpty()) labels += item->name();
        if (!item->description().isEmpty()) labels += item->description();
        if (settings_category *group = dynamic_cast<settings_category*>(item))
            labels += row_labels(group->child_items());
    }
    return labels;
}

// Centres a widget like the settings rows (capped by QSS max-width).
QWidget *centered(QWidget *widget){
    QWidget *wrapper = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(wrapper);
    layout->setContentsMargins(QMargins(0,0,0,0));
    layout->setSpacing(0);
    layout->addStretch(0);
    layout->addWidget(widget, 1);
    layout->addStretch(0);
    return wrapper;
}

// Mirrors the control's show/hide onto its row and its enabled state onto the row's labels.
// The row is never disabled: under a disabled parent, the control's setEnabled(true) is a silent no-op.
class RowSync : public QObject {
public:
    RowSync(QWidget *control, QWidget *row, QList<QWidget*> labels, std::function<void()> visibility_changed)
        : QObject(row), control(control), row(row), labels(labels), visibility_changed(visibility_changed)
    {
        if (control->testAttribute(Qt::WA_WState_ExplicitShowHide) && control->testAttribute(Qt::WA_WState_Hidden))
            row->hide();
        sync_enabled();
        control->installEventFilter(this);
    }
protected:
    bool eventFilter(QObject *, QEvent *event) override {
        if (event->type() == QEvent::ShowToParent || event->type() == QEvent::HideToParent) {
            bool shown = event->type() == QEvent::ShowToParent;
            if (row->isHidden() == shown) {
                row->setVisible(shown);
                if (visibility_changed) visibility_changed();
            }
        }
        else if (event->type() == QEvent::EnabledChange) {
            sync_enabled();
        }
        return false;
    }
private:
    void sync_enabled(){
        foreach (QWidget *label, labels) label->setEnabled(control->isEnabled());
    }
    QWidget *control, *row;
    QList<QWidget*> labels;
    std::function<void()> visibility_changed;
};

struct category_info { QString id, title, icon; };

// Tree order. Plugins only name the ID; unknown IDs land in "other".
const QList<category_info> categories = {
    {"about", "About", "help-about"},
    {"appearance", "Appearance", "preferences-desktop-theme"},
    {"desktop", "Desktop & Panel", "preferences-desktop"},
    {"displays", "Displays", "preferences-desktop-display"},
    {"input", "Input & Hotkeys", "preferences-desktop-keyboard"},
    {"notifications", "Notifications", "preferences-desktop-notifications"},
    {"power", "Power & Lock", "system-lock-screen"},
    {"session", "Session & Startup", "preferences-system-session"},
    {"system", "System", "preferences-system"},
    {"other", "Other", "preferences-other"},
};

}

// Keeps rows whose search text contains every query word; ancestors of matches stay via recursive filtering.
class SearchFilterModel : public QSortFilterProxyModel {
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    void set_query(const QString &query){
        words = query_words(query);
        invalidateRowsFilter();
    }
protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override {
        if (words.isEmpty()) return true;
        QString text = sourceModel()->index(row, 0, parent).data(SearchTextRole).toString();
        if (text.isEmpty()) return false;
        foreach (const QString &word, words)
            if (!text.contains(word)) return false;
        return true;
    }
private:
    QStringList words;
};

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
    proxy = new SearchFilterModel(this);
    proxy->setSourceModel(model);
    proxy->setRecursiveFilteringEnabled(true);

    // Spacing goes on a wrapper: QLineEdit places its clear button ignoring its own QSS margin.
    QFrame *search_box = new QFrame;
    search_box->setObjectName("SearchBox");
    QVBoxLayout *search_layout = new QVBoxLayout(search_box);
    search_layout->setContentsMargins(QMargins(0,0,0,0));
    search_field = new QLineEdit;
    search_layout->addWidget(search_field);
    search_field->setObjectName("SearchField");
    search_field->setPlaceholderText("Search");
    search_field->setClearButtonEnabled(true);
    search_field->installEventFilter(this);
    connect(search_field, &QLineEdit::textChanged, this, &SettingsManager::search_changed);
    QShortcut *find_shortcut = new QShortcut(QKeySequence::Find, this);
    connect(find_shortcut, &QShortcut::activated, this, [this]{
        search_field->setFocus();
        search_field->selectAll();
    });

    tree = new QTreeView;
    tree->setObjectName("CategoryTree");
    tree->setModel(proxy);
    tree->setHeaderHidden(true);
    tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tree->setExpandsOnDoubleClick(false);
    tree->setIconSize(QSize(22, 22));
    tree->setMinimumWidth(200);
    tree->setItemDelegate(new UntintedIconDelegate(tree));
    tree->viewport()->installEventFilter(this);
    connect(tree, &QTreeView::clicked, this, &SettingsManager::open_index);
    connect(tree, &QTreeView::activated, this, &SettingsManager::open_index);
    connect(tree, &QTreeView::expanded, this, &SettingsManager::collapse_others);

    QFrame *sidebar = new QFrame;
    sidebar->setObjectName("Sidebar");
    QVBoxLayout *sidebar_layout = new QVBoxLayout(sidebar);
    sidebar_layout->setContentsMargins(QMargins(0,0,0,0));
    sidebar_layout->setSpacing(0);
    sidebar_layout->addWidget(search_box);
    sidebar_layout->addWidget(tree, 1);

    hlayout->addWidget(sidebar);
    hlayout->addLayout(stacked_layout, 1);

    this->resize(850,600);
    XdgActivation::instance(); // binds asynchronously; must be ready by the first OpenPage
    QTimer::singleShot(0, this, &SettingsManager::load_settings_ui);
}

SettingsManager::~SettingsManager(){}

bool SettingsManager::eventFilter(QObject *watched, QEvent *event){
    if (watched == search_field && event->type() == QEvent::KeyPress) {
        int key = static_cast<QKeyEvent*>(event)->key();
        if (key == Qt::Key_Escape && !search_field->text().isEmpty()) {
            search_field->clear();
            return true;
        }
        if (key == Qt::Key_Down) {
            tree->setFocus();
            return true;
        }
        if (key == Qt::Key_Return || key == Qt::Key_Enter) {
            open_index(tree->currentIndex());
            return true;
        }
    }
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
    loaded = true;
    open_path(initial_path);
}

void SettingsManager::OpenPage(const QString &path, const QString &activation_token){
    if (!path.isEmpty()) {
        if (!loaded) {
            initial_path = path;
        } else {
            search_field->clear();
            open_path(path);
        }
    }
    showNormal();
    raise();
    XdgActivation::instance()->activateWindow(this, activation_token);
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
            connect(page, &settings_page::open_requested, this, [this](QString path){
                search_field->clear(); // a filtered-out target has no proxy index
                open_path(path);
            });
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

    foreach (settings_page *page, pages) {
        QStandardItem *item = path_items.value(page->path());
        if (item && item->data(PageIdRole).toUuid() == page->id())
            set_search_text(item, page);
    }
}

void SettingsManager::set_search_text(QStandardItem *item, settings_page *page){
    QStandardItem *cat = item;
    while (cat->parent()) cat = cat->parent();
    QStringList base = QStringList{page->name(), cat->text()} + page->keywords();
    // Rows built on open (Hotkeys, Autostart, Displays) are only found through title and keywords.
    QStringList full = base + row_labels(page->child_items());
    item->setData(base.join('\n').toLower(), BaseSearchTextRole);
    item->setData(full.join('\n').toLower(), SearchTextRole);
}

void SettingsManager::open_path(QString path){
    while (path.endsWith('/')) path.chop(1);
    QStandardItem *item = path_items.value(path);
    if (!item && !path.isEmpty())
        qWarning() << "Unknown settings page:" << path;
    if (!item)
        item = model->item(0);
    if (!item) return;
    open_index(proxy->mapFromSource(item->index()));
}

void SettingsManager::search_changed(const QString &text){
    proxy->set_query(text);
    if (!text.trimmed().isEmpty()) {
        tree->expandAll();
        QModelIndex match = first_match();
        if (match.isValid())
            tree->setCurrentIndex(match);
        else
            tree->selectionModel()->clear();
        return;
    }
    tree->collapseAll();
    QModelIndex shown = proxy->mapFromSource(shown_index);
    open_index(shown.isValid() ? shown : proxy->index(0, 0));
}

QModelIndex SettingsManager::first_match(){
    QModelIndex index = proxy->index(0, 0);
    while (index.isValid() && !index.data(PageIdRole).isValid())
        index = proxy->index(0, 0, index);
    return index;
}

// Arrow keys only move the current row; click and Enter open it.
void SettingsManager::open_index(QModelIndex index){
    if (!index.isValid()) return;
    while (!index.data(PageIdRole).isValid() && proxy->hasChildren(index))
        index = proxy->index(0, 0, index);
    tree->setCurrentIndex(index);

    bool searching = !search_field->text().trimmed().isEmpty();
    if (!searching) {
        QModelIndex top = index;
        while (top.parent().isValid()) top = top.parent();
        collapse_others(top);
        for (QModelIndex p = index.parent(); p.isValid(); p = p.parent())
            tree->expand(p);
        if (proxy->hasChildren(index))
            tree->expand(index);
    }

    settings_page *page = page_hash.value(index.data(PageIdRole).toUuid());
    if (!page) return;
    QModelIndex source = proxy->mapToSource(index);
    if (source != shown_index) {
        shown_index = source;
        open_page(page);
    }
    if (searching)
        highlight_match(index);
}

void SettingsManager::collapse_others(const QModelIndex &index){
    if (index.parent().isValid() || !search_field->text().trimmed().isEmpty()) return;
    for (int row = 0; row < proxy->rowCount(); row++)
        if (row != index.row())
            tree->collapse(proxy->index(row, 0));
}

void SettingsManager::highlight_match(const QModelIndex &page_index){
    // Only when the query needs a row label to match, i.e. title, category and keywords don't cover it.
    QString base = page_index.data(BaseSearchTextRole).toString();
    QStringList words;
    foreach (const QString &word, query_words(search_field->text()))
        if (!base.contains(word)) words += word;
    if (words.isEmpty()) return;

    QPointer<QWidget> best;
    int best_score = 0;
    for (const auto &row : row_frames.value(page_index.data(PageIdRole).toUuid())) {
        int score = 0;
        foreach (const QString &word, words)
            if (row.first.contains(word)) score++;
        if (score > best_score && row.second) {
            best = row.second;
            best_score = score;
        }
    }
    QPointer<QScrollArea> area = qobject_cast<QScrollArea*>(stacked_layout->currentWidget());
    if (!best || !area) return;

    // After the new page's layout has run, or ensureWidgetVisible sees stale geometry.
    QTimer::singleShot(0, best, [best, area]{
        if (area) area->ensureWidgetVisible(best, 0, 50);
        settingsrow::flash(best, "searchmatch", true, 1500);
    });
}

void SettingsManager::open_page(settings_page *page){
    page->notify_opened();
    if (page->child_items().isEmpty()) return;
    // Once only: a second run would rebuild from widgets the first one's page owns.
    connect(page, &settings_category::updated, this, &SettingsManager::update_widgets, Qt::UniqueConnection);
    display_widgets(page->id(), page->child_items());
}

void SettingsManager::update_widgets(QUuid parent_id, QList<settings_item*> items){
    row_frames.remove(parent_id);
    delete stack_hash.take(parent_id);
    display_widgets(parent_id, items);
}

void SettingsManager::display_widgets(QUuid parent_id, QList<settings_item*> items){
    if(stack_hash.contains(parent_id)){
        stacked_layout->setCurrentWidget(stack_hash[parent_id]);
        return;
    }

    QVBoxLayout *page_layout = new QVBoxLayout();
    foreach(settings_item* item, items){
        if (settings_widget *widget_item = dynamic_cast<settings_widget*>(item)) {
            page_layout->addWidget(widget_item->is_custom() ? centered(widget_item->widget()) : create_control(parent_id, widget_item));
            continue;
        }
        if (settings_widget_group *widget_group = dynamic_cast<settings_widget_group*>(item))
            page_layout->addWidget(create_group(parent_id, widget_group));
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

QWidget* SettingsManager::create_group(QUuid page_id, settings_widget_group *group){
    QWidget *column = new QWidget;
    QVBoxLayout *column_layout = new QVBoxLayout(column);
    column_layout->setContentsMargins(QMargins(0,0,0,0));
    column_layout->setSpacing(0);

    QFrame *frame = new QFrame;
    frame->setObjectName("WidgetGroup");
    if (!group->name().isEmpty()) {
        QLabel *title = new QLabel(group->name());
        title->setObjectName("GroupTitle");
        column_layout->addWidget(title);
        row_frames[page_id].append({group->name().toLower(), frame});
    }
    column_layout->addWidget(frame);

    QVBoxLayout *group_layout = new QVBoxLayout(frame);
    group_layout->setContentsMargins(QMargins(0,0,0,0));
    group_layout->setSpacing(0);
    // Outer row widgets (children of column, so the lambda can't outlive them); their ControlWidget holds groupposition.
    auto rows = std::make_shared<QList<QWidget*>>();
    // Positions skip hidden rows; a group with none visible hides with its title.
    auto update_positions = [column, rows]{
        QList<QWidget*> visible;
        foreach (QWidget *row, *rows)
            if (!row->isHidden()) visible.append(row);
        column->setVisible(!visible.isEmpty() || rows->isEmpty());
        for (int i = 0; i < visible.size(); i++) {
            QString position = visible.size() == 1 ? "only" : i == 0 ? "first" : i == visible.size() - 1 ? "last" : "middle";
            QWidget *control = visible[i]->findChild<QWidget*>("ControlWidget");
            if (control->property("groupposition").toString() == position) continue;
            control->setProperty("groupposition", position);
            settingsrow::repolish(control);
        }
    };
    foreach (settings_item *sub_item, group->child_items()) {
        settings_widget *sub_widget = dynamic_cast<settings_widget*>(sub_item);
        if (!sub_widget) continue;
        if (sub_widget->is_custom()) {
            group_layout->addWidget(sub_widget->widget());
            continue;
        }
        QWidget *row = create_control(page_id, sub_widget, update_positions);
        group_layout->addWidget(row);
        rows->append(row);
    }

    update_positions();
    return column;
}

QWidget* SettingsManager::create_control(QUuid page_id, settings_widget* item, std::function<void()> visibility_changed){
    QFrame *control_widget = new QFrame;
    control_widget->setObjectName("ControlWidget");
    QHBoxLayout *h_layout = new QHBoxLayout(control_widget);
    h_layout->setContentsMargins(QMargins(0,0,0,0));
    h_layout->setSpacing(0);
    QList<QWidget*> labels;
    if (!item->name().isEmpty()) {
        QVBoxLayout *label_layout = new QVBoxLayout;
        label_layout->setContentsMargins(QMargins(0,0,0,0));
        label_layout->setSpacing(0);
        QLabel *name_label = new QLabel(item->name());
        label_layout->addWidget(name_label);
        labels.append(name_label);
        if (!item->description().isEmpty()) {
            QLabel *description_label = new QLabel(item->description());
            description_label->setObjectName("RowDescription");
            description_label->setWordWrap(true);
            label_layout->addWidget(description_label);
            labels.append(description_label);
        }
        h_layout->addLayout(label_layout, 1);
        row_frames[page_id].append({(item->name() + '\n' + item->description()).toLower(), control_widget});
    }
    // Unnamed rows are full width.
    h_layout->addWidget(item->widget(), item->name().isEmpty() ? 1 : 0);

    QWidget *row = centered(control_widget);
    // After reparenting: setParent hides a visible widget, which would read as the page hiding it.
    new RowSync(item->widget(), row, labels, visibility_changed);
    return row;
}
