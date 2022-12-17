/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
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

#ifndef WIDGETSENSORCONF_H
#define WIDGETSENSORCONF_H

#include <QWidget>
#include <QColorDialog>

namespace Ui {
class WidgetSensorConf;
}

class WidgetSensorConf : public QWidget
{
    Q_OBJECT
    
public:
    explicit WidgetSensorConf(const QStringList &list, QWidget *parent = nullptr);
    ~WidgetSensorConf();

signals:
    void settingSaved();

private slots:
    void loadsettings();
    void on_okbt_clicked();

    void on_textRbt_toggled(bool checked);

    void on_backcolorbt_clicked();

private:
    Ui::WidgetSensorConf *ui;
    QStringList sensorlist;
    QColor backcolor;

};

#endif // WIDGETSENSORCONF_H
