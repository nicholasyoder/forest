// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wallpaperwidget.h"
#include "menuanchor.h"

#include <LayerShellQt/Window>
#include <QScreen>
#include <QWindow>

wallpaperwidget::wallpaperwidget(QImage *image, WALLPAPER_MODE imode, QScreen *screen, bool hasicons){
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
    // Icons take keys (rename, Delete, Ctrl+A...) once clicked.
    layer_window->setKeyboardInteractivity(hasicons ? LayerShellQt::Window::KeyboardInteractivityOnDemand
                                                    : LayerShellQt::Window::KeyboardInteractivityNone);
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
    if (event->button() == Qt::RightButton && cmenu) {
        menuanchor::anchorMenuAtPoint(cmenu, this, event->pos());
        cmenu->exec(event->globalPosition().toPoint());
    }
}

void wallpaperwidget::setup_wallpaper(){
    scaledwallpaper = miscutills::get_wallpaper_scaled(wallpaper, imagemode, size());
    update();
}

