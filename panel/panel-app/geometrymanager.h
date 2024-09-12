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

#ifndef GEOMETRYMANAGER_H
#define GEOMETRYMANAGER_H

#include <QWidget>

class GeometryManager : public QObject
{
    Q_OBJECT
public:
    explicit GeometryManager(QWidget *panel);
    ~GeometryManager();

public slots:
    void set_fixed_size(int size);
    void set_panel_position(QString position);
    void set_reserve_screen_space(bool reserve);
    void update_geometry();

signals:

private:
    QWidget* panel_widget = nullptr;
    int fixed_panel_size = 0;
    QString panel_position;
    bool reserve_screen_space = false;
};

#endif // GEOMETRYMANAGER_H
