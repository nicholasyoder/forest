// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wallpaperwidget.h"

#include <LayerShellQt/Window>

wallpaperwidget::wallpaperwidget(QImage *image, WALLPAPER_MODE imode){
    Qt::WindowFlags flags;
    flags |= Qt::FramelessWindowHint;
    flags |= Qt::WindowStaysOnBottomHint;
    setWindowFlags(flags);

    winId(); // force native window creation so windowHandle() is valid
    LayerShellQt::Window *layer_window = LayerShellQt::Window::get(windowHandle());
    layer_window->setLayer(LayerShellQt::Window::LayerBackground);
    layer_window->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorBottom
                              | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));
    // -1, not 0: the background should extend fully behind any panel's
    // reserved exclusive zone (standard wallpaper/lock-screen idiom, see the
    // wlr-layer-shell-unstable-v1 protocol's own set_exclusive_zone doc
    // comment). 0 would mean "respect other surfaces' exclusive zones", so
    // Biome's arrange_layers() would configure this surface against the
    // panel-shrunk usable_area instead of the full output - conflicting with
    // this widget's own setFixedSize(screen->size()) in desktop.cpp and
    // triggering continuous reconfigure/recommit churn (handle_layer_surface_commit
    // reruns arrange_layers() on every commit from any layer surface on the
    // output), which is what was making the cursor jerky whenever the panel
    // reserved space.
    layer_window->setExclusiveZone(-1);
    layer_window->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
    layer_window->setScope("forest-desktop");

    wallpaper = image;
    imagemode = imode;

    setup_wallpaper();
}

void wallpaperwidget::paintEvent(QPaintEvent *){
    QRectF target(0.0, 0.0, width(), height());
    QRectF source(0.0, 0.0, scaledwallpaper->width(), scaledwallpaper->height());
    QPainter painter(this);
    painter.drawImage(target, *scaledwallpaper, source);
}

void wallpaperwidget::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() == Qt::RightButton)
        if (cmenu) cmenu->exec(event->pos());
}

void wallpaperwidget::setup_wallpaper(){
    scaledwallpaper = miscutills::get_wallpaper_scaled(wallpaper, imagemode, size());
    update();
}

