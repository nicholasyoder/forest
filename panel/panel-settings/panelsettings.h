/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2024 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef PANELSETTINGS_H
#define PANELSETTINGS_H

#include <QComboBox>
#include "../../settings/widgets/listwidget.h"
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


class PanelSettings : public QObject, settings_plugin_infterace
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.panel.plugin")
    Q_INTERFACES(settings_plugin_infterace)

public:
    PanelSettings();

    // settings_plugin_infterace functions
    QList<settings_item*> get_settings_items();

public slots:
    void load_behavior_settings();
    void set_behavior_settings();

    void load_applets();
    void set_applets();

    void resize_applet_list();

private:
    QString padwithzeros(int number);
    QComboBox *position_select = nullptr;
    QComboBox *autohide_select = nullptr;
    ListWidget *applet_list_w = nullptr;
    QHash <QString, QString> path_hash;
};

#endif // PANELSETTINGS_H
