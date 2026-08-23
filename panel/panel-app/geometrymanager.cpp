// SPDX-License-Identifier: LGPL-3.0-or-later

#include "geometrymanager.h"

#include <QApplication>
#include <QScreen>
#include <LayerShellQt/Window>

#include "miscutills.h"

GeometryManager::GeometryManager(QWidget *panel) : panel_widget(panel) {
    panel_widget->winId(); // force native window creation so windowHandle() is valid
    layer_window = LayerShellQt::Window::get(panel_widget->windowHandle());
    layer_window->setLayer(LayerShellQt::Window::LayerTop);
    layer_window->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
    layer_window->setScope("forest-panel");

    RunOnce* runner = new RunOnce(2000);
    connect(qApp, &QGuiApplication::screenAdded, runner, &RunOnce::try_activate);
    connect(qApp, &QGuiApplication::screenRemoved, runner, &RunOnce::try_activate);
    connect(qApp->primaryScreen(), &QScreen::geometryChanged, runner, &RunOnce::try_activate);
    connect(runner, &RunOnce::activated, this, &GeometryManager::update_geometry);
}

GeometryManager::~GeometryManager(){
    delete panel_widget;
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

void GeometryManager::update_geometry(){

    // reset fixed size
    panel_widget->setMinimumSize(0,0);
    panel_widget->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);

    // default to sizeHint when no fixed size is set
    if(fixed_panel_size == 0)
        fixed_panel_size = panel_widget->sizeHint().height();

    // Width is never requested here: the panel is anchored to both left and
    // right, so the compositor always assigns the real width via a configure
    // event regardless of what's requested (arrange_layers() in Biome
    // overrides it unconditionally for a double-anchored dimension).
    // update_geometry() runs reactively off pframe's own resized signal
    // (panel::update_panel_size()) as well as output-change signals, so
    // re-requesting a guessed width here on every call fights the
    // compositor's own configure in a resize feedback loop whenever that
    // guess (qApp->primaryScreen()) differs from whichever output the
    // panel's layer surface actually landed on - only reachable on a real
    // multi-monitor setup, not the single-output nested dev loop.
    panel_widget->setFixedHeight(fixed_panel_size);
    if (panel_position == "top")
        layer_window->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));
    else //bottom
        layer_window->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorBottom | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));

    layer_window->setExclusiveZone(reserve_screen_space ? fixed_panel_size : 0);
}
