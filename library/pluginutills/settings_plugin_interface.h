// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGS_PLUGIN_INTERFACE_H
#define SETTINGS_PLUGIN_INTERFACE_H

#include <QObject>
#include <QWidget>
#include <QUuid>

// Base settings item class with name and optional description (used for search)
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

// Group of widgets
class settings_widget_group : public settings_category {
    Q_OBJECT
public:
    settings_widget_group() : settings_category("") {}
};

// Settings item with custom widget
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

/*
// Settings item with custom widget
class settings_widget : public settings_item {
    Q_OBJECT
public:
    settings_widget(QString name, QString description = "", QWidget *widget = nullptr)
        : settings_item(name, description) { set_widget(widget); }
signals:
    void widget_ready(QWidget *widget);
    void widget_request();
public slots:
    // Request item widget
    // Called when item is ready to be shown and widget is needed
    // If item_widget is already prepared, emit widget_ready passing item_widget
    // Other wise emit a widget_request signal to notify the relevent plugin-settings to setup this widget
    void request_widget(){
        if (item_widget)
            emit widget_ready(item_widget);
        else
            emit widget_request();
    }

    void set_widget(QWidget *widget){
        item_widget = widget;
        emit widget_ready(item_widget);
    }

private:
    QWidget *item_widget = nullptr;
};

class settings_control : public settings_widget {
    Q_OBJECT
};
*/

class settings_plugin_infterace {

public:

    // Destructor
    virtual ~settings_plugin_infterace() {}

    // Get settings items
    virtual QList<settings_item*> get_settings_items() = 0;

};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(settings_plugin_infterace, "forest.settings.plugin.interface")
QT_END_NAMESPACE

#endif // SETTINGS_PLUGIN_INTERFACE_H
