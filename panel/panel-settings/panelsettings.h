// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PANELSETTINGS_H
#define PANELSETTINGS_H

#include <QComboBox>
#include <QSpinBox>
#include "listwidget.h"
#include <QDebug>
#include <QDropEvent>
#include <QTimer>

#include "miscutills.h"
#include "../../library/pluginutills/settings_plugin_interface.h"

// ReorderListener is an event filter for QListWidget
// The reordered signal is emitted whenever the list items are reordered via drag and drop
// For some reason QListWidget doesn't have a native signal for this
class ReorderListener: public QObject{
    Q_OBJECT
public:
    ReorderListener(QListWidget *list_widget): QObject() { parent_list_widget = list_widget; }
    bool eventFilter(QObject* object, QEvent* event){
        Q_UNUSED(object);
        if (event->type() == QEvent::ChildAdded) {
            last_items = parent_list_widget->findItems("*", Qt::MatchWildcard);
        }
        else if (event->type() == QEvent::ChildRemoved) {
            QList<QListWidgetItem*> items = parent_list_widget->findItems("*", Qt::MatchWildcard);
            if (last_items != parent_list_widget->findItems("*", Qt::MatchWildcard)){
                last_items = items;
                emit reordered();
            }
        }
        return false;
    }
signals:
    void reordered();
private:
    QListWidget *parent_list_widget = nullptr;
    QList<QListWidgetItem*> last_items;
};


class PanelSettings : public QObject, settings_plugin_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.panel.plugin")
    Q_INTERFACES(settings_plugin_interface)

public:
    PanelSettings();

    QList<settings_page*> pages() override;

public slots:
    void load_applets();
    void set_applets();


private:
    QString padwithzeros(int number);
    ListWidget *applet_list_w = nullptr;
    QHash <QString, QString> path_hash;
};

#endif // PANELSETTINGS_H
