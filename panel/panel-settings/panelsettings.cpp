// SPDX-License-Identifier: LGPL-3.0-or-later

#include "panelsettings.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QFileInfo>
#include <QSettings>
#include <QPluginLoader>
#include <QTimer>
#include <QToolTip>
#include <QPainter>
#include <QMouseEvent>
#include <QHelpEvent>
#include <QStyleOptionToolButton>

#include "panelconfig.h"
#include "settingsbinder.h"
#include "settingsrow.h"

PanelSettings::PanelSettings()
{

}

AppletDelegate::AppletDelegate(QAbstractItemView *view) : QStyledItemDelegate(view), view(view){
    style_source = new QToolButton(view->viewport());
    style_source->setObjectName("AppletSettingsButton");
    style_source->setIcon(QIcon::fromTheme("configure"));
    style_source->hide();
    view->viewport()->setMouseTracking(true);
    view->viewport()->installEventFilter(this);
}

// Right-aligned; the size includes the QSS box model (padding, border, margin).
QRect AppletDelegate::gear_rect(const QRect &row) const {
    style_source->ensurePolished();
    const QSize size = style_source->sizeHint();
    return QRect(QPoint(row.right() - size.width() + 1, row.center().y() - size.height() / 2), size);
}

QSize AppletDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    if (!index.data(SettingsPathRole).toString().isEmpty())
        size.setHeight(qMax(size.height(), gear_rect(QRect()).height()));
    return size;
}

// Delegates get no hover position, so track which row's gear the pointer is over.
bool AppletDelegate::eventFilter(QObject *object, QEvent *event){
    if (object == view->viewport()) {
        const QEvent::Type type = event->type();
        if (type == QEvent::MouseMove || type == QEvent::MouseButtonPress || type == QEvent::MouseButtonRelease || type == QEvent::Leave) {
            QPersistentModelIndex over;
            if (type != QEvent::Leave) {
                const QPoint pos = static_cast<QMouseEvent*>(event)->position().toPoint();
                const QModelIndex index = view->indexAt(pos);
                if (index.isValid() && !index.data(SettingsPathRole).toString().isEmpty()
                        && gear_rect(view->visualRect(index)).contains(pos))
                    over = index;
            }
            QPersistentModelIndex now_pressed = pressed;
            if (type == QEvent::MouseButtonPress) now_pressed = over;
            else if (type == QEvent::MouseButtonRelease || type == QEvent::Leave) now_pressed = QPersistentModelIndex();
            if (over != hovered || now_pressed != pressed) {
                hovered = over;
                pressed = now_pressed;
                view->viewport()->update();
            }
        }
    }
    return QStyledItemDelegate::eventFilter(object, event);
}

void AppletDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);
    if (index.data(SettingsPathRole).toString().isEmpty()) return;

    QStyleOptionToolButton opt;
    opt.initFrom(style_source);
    opt.rect = gear_rect(option.rect);
    opt.state = QStyle::State_Enabled | QStyle::State_Raised;
    if (hovered == index) opt.state |= QStyle::State_MouseOver;
    if (pressed == index) opt.state |= QStyle::State_Sunken;
    opt.icon = style_source->icon();
    opt.iconSize = style_source->iconSize();
    opt.toolButtonStyle = Qt::ToolButtonIconOnly;
    opt.features = QStyleOptionToolButton::None;
    style_source->style()->drawComplexControl(QStyle::CC_ToolButton, &opt, painter, style_source);
}

bool AppletDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index){
    const QString path = index.data(SettingsPathRole).toString();
    if (!path.isEmpty() && (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonPress)) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton && gear_rect(option.rect).contains(me->position().toPoint())) {
            if (event->type() == QEvent::MouseButtonRelease) emit gearClicked(path);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

bool AppletDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index){
    if (event->type() == QEvent::ToolTip && !index.data(SettingsPathRole).toString().isEmpty()
            && gear_rect(option.rect).contains(event->pos())) {
        QToolTip::showText(event->globalPos(), index.data(Qt::DisplayRole).toString() + " Settings", view);
        return true;
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

QList<settings_page*> PanelSettings::pages(){
    panel_page = new settings_page("desktop/panel", "Panel", "preferences-desktop");
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
    applet_list_w->setObjectName("AppletList");
    applet_list_w->setDragDropMode(QAbstractItemView::InternalMove);
    AppletDelegate *delegate = new AppletDelegate(applet_list_w);
    connect(delegate, &AppletDelegate::gearClicked, panel_page, [this](const QString &path){ emit panel_page->open_requested(path); });
    applet_list_w->setItemDelegate(delegate);
    connect(applet_list_w, &QListWidget::itemChanged, this, &PanelSettings::set_applets);
    ReorderListener *rl = new ReorderListener(applet_list_w);
    connect(rl, &ReorderListener::reordered, this, &PanelSettings::set_applets);
    applet_list_w->installEventFilter(rl);
    applets_group->add_child(new settings_widget("", "", applet_list_w));

    return {panel_page};
}

void PanelSettings::load_applets(){
    // itemChanged would run set_applets on the half-built list and overwrite the config.
    QSignalBlocker blocker(applet_list_w);
    applet_list_w->clear();

    QSettings settings("Forest", "Panel");
    settings.beginGroup("plugins");

    foreach(QString key, settings.childGroups()){
        QString path = settings.value(key+"/path").toString();
        bool enabled = settings.value(key+"/enabled", false).toBool();
        QString name = "Separator";
        QString settings_path;
        if (path != "separator"){
            // Metadata only: instantiating would construct the applet in this process.
            QJsonObject info = QPluginLoader(path).metaData().value("MetaData").toObject();
            name = info.value("name").toString();
            if (name.isEmpty()) {
                // Keep the row so a save doesn't erase it (plugin removed or unreadable).
                qWarning() << "Not a panel applet:" << path;
                name = tr("%1 (unavailable)").arg(QFileInfo(path).fileName());
            }
            else settings_path = info.value("settings").toString();
        }
        QListWidgetItem *item = new QListWidgetItem(name);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
        item->setData(AppletDelegate::SettingsPathRole, settings_path);
        applet_list_w->addItem(item);
        item->setData(AppletDelegate::ConfigPathRole, path);
    }
}

void PanelSettings::set_applets(){
    QSettings settings("Forest", "Panel");
    settings.beginGroup("plugins");
    settings.remove("");

    for(int i = 0; i < applet_list_w->count(); i++){
        settings.beginGroup("plug-"+padwithzeros(i));
        settings.setValue("path", applet_list_w->item(i)->data(AppletDelegate::ConfigPathRole).toString());
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
