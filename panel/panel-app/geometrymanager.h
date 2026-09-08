// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GEOMETRYMANAGER_H
#define GEOMETRYMANAGER_H

#include <QList>
#include <QWidget>

class QScreen;

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
    void handle_screen_change();

    // The caller's content widget (panel or HiddenPanel) - never itself a
    // top-level. Reparented into a fresh shell on each build_shell(), so its
    // state (loaded plugins, DBus registration, ...) survives a rebuild.
    QWidget* panel_widget = nullptr;

    // The actual top-level, layer-shell-bound window - a bare translucent/
    // frameless frame around panel_widget. See build_shell()'s comment.
    QWidget* shell = nullptr;

    LayerShellQt::Window* layer_window = nullptr;
    int fixed_panel_size = 0;
    QString panel_position;
    bool reserve_screen_space = false;

    // Screens seen as of the last handle_screen_change() - see its comment.
    QList<QScreen*> tracked_screens;
};

#endif // GEOMETRYMANAGER_H
