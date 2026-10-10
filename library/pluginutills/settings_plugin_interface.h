// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGS_PLUGIN_INTERFACE_H
#define SETTINGS_PLUGIN_INTERFACE_H

#include <QObject>
#include <QWidget>
#include <QUuid>
#include <QStringList>

// Base settings item class with name and optional description
class settings_item : public QObject {
    Q_OBJECT
public:
    settings_item(QString name, QString description = ""){ set_name(name); set_description(description); item_id = QUuid::createUuid(); }
    void set_name(QString name){ item_name = name; }
    void set_description(QString description){ item_description = description; }
    QString name(){ return item_name; }
    QString description(){ return item_description; }
    QUuid id(){ return item_id; }
private:
    QString item_name;
    QString item_description;
    QUuid item_id;
};

// Settings category with icon and child settings items
class settings_category : public settings_item {
    Q_OBJECT
public:
    settings_category(QString name, QString description = "", QString icon = "")
        : settings_item(name, description) { set_icon(icon); }
    ~settings_category(){ clear(); }
    void set_icon(QString icon){ item_icon = icon; }
    void add_child(settings_item *child){ item_children.append(child); }
    void clear(){ foreach(settings_item* item, item_children){ delete item; } item_children.clear(); }
    QString icon(){ return item_icon; }
    QList<settings_item*> child_items(){ return item_children; }
signals:
    void opened();
    void updated(QUuid id, QList<settings_item*> new_child_items);
public slots:
    void notify_opened(){ emit opened(); }
    void notify_updated(){ emit updated(id(), child_items()); }
private:
    QString item_icon;
    QList<settings_item*> item_children;
};

// Group of widgets; the optional title is shown above it and searched.
class settings_widget_group : public settings_category {
    Q_OBJECT
public:
    settings_widget_group(QString title = "") : settings_category(title) {}
};

// Settings row: name on the left (description below it), widget on the right; unnamed rows are full width.
// The row follows the widget's setVisible / setEnabled. Custom widgets are placed as-is.
class settings_widget : public settings_item {
    Q_OBJECT
public:
    settings_widget(QString name, QString description = "", QWidget *widget = nullptr, bool custom = false)
        : settings_item(name, description) { set_widget(widget); set_custom(custom); }
    ~settings_widget(){ delete item_widget; }
    QWidget* widget(){ return item_widget; }
    bool is_custom(){ return custom_widget; }
    void set_widget(QWidget *widget){ item_widget = widget; }
    void set_custom(bool custom){ custom_widget = custom; }
private:
    QWidget *item_widget = nullptr;
    bool custom_widget = false;
};

// Page in the settings tree. Path is "<category>/[<parent>/...]<page>": deep link and placement.
class settings_page : public settings_category {
    Q_OBJECT
public:
    settings_page(QString path, QString name, QString icon = "", int order = 0)
        : settings_category(name, "", icon), page_path(path), page_order(order) {}
    QString path(){ return page_path; }
    int order(){ return page_order; }
    QStringList keywords(){ return page_keywords; }
    void set_keywords(QStringList keywords){ page_keywords = keywords; }
private:
    QString page_path;
    int page_order;
    QStringList page_keywords;
};

class settings_plugin_interface {
public:
    virtual ~settings_plugin_interface() {}
    virtual QList<settings_page*> pages() = 0;
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(settings_plugin_interface, "forest.settings.plugin.interface/2")
QT_END_NAMESPACE

#endif // SETTINGS_PLUGIN_INTERFACE_H
