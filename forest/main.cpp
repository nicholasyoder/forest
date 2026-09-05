// SPDX-License-Identifier: LGPL-3.0-or-later

#include "forestxcbeventfilter.h"
#include "flogger.h"
#include <QApplication>
#include <LayerShellQt/Shell>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    // useLayerShell() just sets QT_WAYLAND_SHELL_INTEGRATION=layer-shell,
    // which the Wayland platform plugin only reads once, at QApplication
    // construction above - calling it after is a no-op for the process-wide
    // default. Left as-is deliberately, not fixed: LayerShellQt::Window::get()
    // (used explicitly by the panel/wallpaper) bypasses the process default
    // anyway, and every *other* window currently getting the plain xdg-shell
    // integration by accident is exactly what lets panel popups attach to the
    // panel's layer surface via zwlr_layer_surface_v1.get_popup (see
    // panel-library/popup.h). QWaylandLayerShellIntegration::createShellSurface()
    // is unconditional (no branching on window type), so fixing the ordering
    // would make it the real default for every window in the process,
    // defaulting each to a full-output-sized layer surface unless explicitly
    // reconfigured - not a free fix. Revisit only alongside a deliberate move
    // to a layer-shell-surface-per-popup design.
    LayerShellQt::Shell::useLayerShell();
    // The env var set above is a no-op for this process (see comment) but
    // otherwise leaks into every child forest spawns (hotkeys, main menu),
    // making their windows layer surfaces too - confirmed with pcmanfm-qt
    // opening full-output-sized and undecorated. Clear it to stop the leak.
    qunsetenv("QT_WAYLAND_SHELL_INTEGRATION");
    FLogger::install("forest");
    forest w;

    forestXcbEventFilter *eventfilter = new forestXcbEventFilter;
    eventfilter->f = &w;
    a.installNativeEventFilter(eventfilter);

    w.setup();

    return a.exec();
}
