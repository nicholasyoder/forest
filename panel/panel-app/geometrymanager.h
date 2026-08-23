// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GEOMETRYMANAGER_H
#define GEOMETRYMANAGER_H

#include <QWidget>

namespace LayerShellQt {
class Window;
}

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
    LayerShellQt::Window* layer_window = nullptr;
    int fixed_panel_size = 0;
    QString panel_position;
    bool reserve_screen_space = false;
};

#endif // GEOMETRYMANAGER_H
