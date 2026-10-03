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
    void set_collapsed(bool collapsed);

public:
    bool is_collapsed() const { return collapsed; }
    QWidget *shell_widget() const { return shell; }

signals:

private:
    void build_shell();
    void rebuild_shell();
    void handle_geometry_change();
    void apply_collapsed();

    // Content widget (the panel); reparented into each new shell so its state survives.
    QWidget* panel_widget = nullptr;

    // Layer-shell top-level wrapping panel_widget. See build_shell().
    QWidget* shell = nullptr;
    QPointer<QScreen> shell_screen;

    LayerShellQt::Window* layer_window = nullptr;
    int fixed_panel_size = 0;
    QString panel_position;
    bool reserve_screen_space = false;
    bool collapsed = false;
};

#endif // GEOMETRYMANAGER_H
