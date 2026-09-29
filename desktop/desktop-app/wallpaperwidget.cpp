// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wallpaperwidget.h"

#include <LayerShellQt/Window>
#include <QScreen>
#include <QWindow>

wallpaperwidget::wallpaperwidget(QImage *image, WALLPAPER_MODE imode, QScreen *screen){
    Qt::WindowFlags flags;
    flags |= Qt::FramelessWindowHint;
    flags |= Qt::WindowStaysOnBottomHint;
    setWindowFlags(flags);

    winId(); // force native window creation so windowHandle() is valid
    windowHandle()->setScreen(screen);
    resize(screen->size()); // pre-configure size: a layout-sized (0x0) top-level never maps
    LayerShellQt::Window *layer_window = LayerShellQt::Window::get(windowHandle());
    layer_window->setLayer(LayerShellQt::Window::LayerBackground);
    layer_window->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorBottom
                              | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));
    // -1: extend behind panels' exclusive zones instead of being shrunk by them.
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

