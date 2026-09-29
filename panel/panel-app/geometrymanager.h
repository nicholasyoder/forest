// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GEOMETRYMANAGER_H
#define GEOMETRYMANAGER_H

#include <QPointer>
#include <QScreen>
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
    void build_shell();
    void rebuild_shell();
    void handle_geometry_change();

    // Content widget (panel or HiddenPanel); reparented into each new shell so its state survives.
    QWidget* panel_widget = nullptr;

    // Layer-shell top-level wrapping panel_widget. See build_shell().
    QWidget* shell = nullptr;
    QPointer<QScreen> shell_screen;

    LayerShellQt::Window* layer_window = nullptr;
    int fixed_panel_size = 0;
    QString panel_position;
    bool reserve_screen_space = false;
};

#endif // GEOMETRYMANAGER_H
