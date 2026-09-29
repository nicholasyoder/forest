// SPDX-License-Identifier: LGPL-3.0-or-later

#include "geometrymanager.h"

#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QWindow>
#include <LayerShellQt/Window>

#include "miscutills.h"

GeometryManager::GeometryManager(QWidget *panel) : panel_widget(panel) {
    build_shell();

    ScreenTracker *tracker = new ScreenTracker(this);
    connect(tracker, &ScreenTracker::screens_replaced, this, &GeometryManager::rebuild_shell);
    connect(tracker, &ScreenTracker::geometry_changed, this, &GeometryManager::handle_geometry_change);
}

// panel_widget is never the top-level itself: LayerShellQt only attaches the
// layer-shell role once per QWindow, so moving outputs needs a fresh shell.
void GeometryManager::build_shell() {
    shell = new QWidget();
    shell->setAttribute(Qt::WA_TranslucentBackground);
    shell->setWindowFlags(Qt::FramelessWindowHint);

    auto *layout = new QVBoxLayout(shell);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(panel_widget);
    panel_widget->show();

    shell->winId(); // force native window creation so windowHandle() is valid
    shell_screen = ScreenTracker::primary();
    if (shell_screen)
        shell->windowHandle()->setScreen(shell_screen);
    layer_window = LayerShellQt::Window::get(shell->windowHandle());
    layer_window->setLayer(LayerShellQt::Window::LayerTop);
    layer_window->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
    layer_window->setScope("forest-panel");
}

void GeometryManager::rebuild_shell() {
    QWidget *old_shell = shell;
    build_shell(); // reparents panel_widget out of old_shell
    update_geometry();
    old_shell->deleteLater();
}

void GeometryManager::handle_geometry_change() {
    if (ScreenTracker::primary() != shell_screen)
        rebuild_shell();
    else
        update_geometry();
}

GeometryManager::~GeometryManager(){
    delete shell; // cascades to panel_widget, its sole child
    shell = nullptr;
    panel_widget = nullptr;
}

void GeometryManager::set_fixed_size(int size){
    fixed_panel_size = size;
}

void GeometryManager::set_panel_position(QString position){
    panel_position = position;
}

void GeometryManager::set_reserve_screen_space(bool reserve){
    reserve_screen_space = reserve;
}

// Also shows the shell (panel_widget's show() only affects the child).
void GeometryManager::update_geometry(){

    // reset fixed size
    shell->setMinimumSize(0,0);
    shell->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);

    // default to sizeHint when no fixed size is set
    if(fixed_panel_size == 0)
        fixed_panel_size = panel_widget->sizeHint().height();

    // Width comes from the compositor (anchored left+right); setting it here
    // fights its configure in a resize loop.
    shell->setFixedHeight(fixed_panel_size);
    if (panel_position == "top")
        layer_window->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));
    else //bottom
        layer_window->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorBottom | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));

    layer_window->setExclusiveZone(reserve_screen_space ? fixed_panel_size : 0);
    shell->show();
}
