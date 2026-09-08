// SPDX-License-Identifier: LGPL-3.0-or-later

#include "geometrymanager.h"

#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <LayerShellQt/Window>

#include "miscutills.h"

GeometryManager::GeometryManager(QWidget *panel) : panel_widget(panel) {
    build_shell();

    tracked_screens = qApp->screens();

    RunOnce* runner = new RunOnce(2000);
    connect(qApp, &QGuiApplication::screenAdded, runner, &RunOnce::try_activate);
    connect(qApp, &QGuiApplication::screenRemoved, runner, &RunOnce::try_activate);
    connect(qApp->primaryScreen(), &QScreen::geometryChanged, runner, &RunOnce::try_activate);
    connect(runner, &RunOnce::activated, this, &GeometryManager::handle_screen_change);
}

// Builds a fresh top-level shell, binds it to the layer-shell protocol, and
// reparents panel_widget into it (a real Qt reparent, not a no-op).
// panel_widget is never itself the top-level - LayerShellQt only attaches
// the layer-shell role the first time it's called for a given QWindow, so
// recreating a widget's own native window in place comes back unanchored
// instead of properly layer-shelled.
void GeometryManager::build_shell() {
    shell = new QWidget();
    shell->setAttribute(Qt::WA_TranslucentBackground);
    shell->setWindowFlags(Qt::FramelessWindowHint);

    auto *layout = new QVBoxLayout(shell);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(panel_widget);
    panel_widget->show();

    shell->winId(); // force native window creation so windowHandle() is valid
    layer_window = LayerShellQt::Window::get(shell->windowHandle());
    layer_window->setLayer(LayerShellQt::Window::LayerTop);
    layer_window->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
    layer_window->setScope("forest-panel");
}

// A layer-shell surface is destroyed along with its output - including
// when a compositor puts a monitor to sleep by disabling its connector
// (some hardware bounces its HPD line when that happens, which looks like
// a real disconnect+reconnect), not just on an actual unplug. Either way
// Qt replaces the old QScreen with a new pointer, so any output/screen
// change here has to rebuild the surface. Comparing by pointer identity
// (not geometry) is what desktop.cpp's handleScreenChange() does too -
// geometry alone would wrongly call a same-geometry replacement "unchanged".
void GeometryManager::handle_screen_change() {
    bool all_tracked = qApp->screens().length() == tracked_screens.length();
    if (all_tracked) {
        foreach (QScreen *screen, qApp->screens()){
            if (!tracked_screens.contains(screen)){
                all_tracked = false;
                break;
            }
        }
    }

    if (!all_tracked) {
        QWidget *old_shell = shell;
        build_shell(); // reparents panel_widget out of old_shell and into the new one
        update_geometry(); // stages anchors and shows the new shell - see its comment
        old_shell->deleteLater(); // panel_widget already moved out, so this only frees the dead shell
        tracked_screens = qApp->screens();
    } else {
        update_geometry();
    }
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

// Also shows the shell - the one call both the initial-setup and recovery
// paths always run through after anchors are staged, so it's the natural
// place to make the top-level visible (panel_widget's own show() calls
// only affect the child, never the top-level itself).
void GeometryManager::update_geometry(){

    // reset fixed size
    shell->setMinimumSize(0,0);
    shell->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);

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
    shell->setFixedHeight(fixed_panel_size);
    if (panel_position == "top")
        layer_window->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));
    else //bottom
        layer_window->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorBottom | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));

    layer_window->setExclusiveZone(reserve_screen_space ? fixed_panel_size : 0);
    shell->show();
}
