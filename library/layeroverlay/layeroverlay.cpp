// SPDX-License-Identifier: LGPL-3.0-or-later

#include "layeroverlay.h"

#include <QGuiApplication>
#include <QPainter>
#include <QWindow>

layeroverlay::layeroverlay(const QColor &color, LayerShellQt::Window::Layer layer, const QString &scope, QScreen *screen, bool passInput) : color(color){
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(passInput ? Qt::FramelessWindowHint | Qt::WindowTransparentForInput : Qt::FramelessWindowHint);

    winId(); // force native window creation so windowHandle() is valid
    windowHandle()->setScreen(screen);
    LayerShellQt::Window *layer_window = LayerShellQt::Window::get(windowHandle());
    layer_window->setLayer(layer);
    layer_window->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorBottom
                              | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));
    layer_window->setExclusiveZone(-1);
    layer_window->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
    layer_window->setScope(scope);
}

QList<layeroverlay*> layeroverlay::showOnAllScreens(const QColor &color, LayerShellQt::Window::Layer layer, const QString &scope, bool passInput){
    QList<layeroverlay*> overlays;
    for (QScreen *screen : QGuiApplication::screens()){
        layeroverlay *overlay = new layeroverlay(color, layer, scope, screen, passInput);
        overlay->show();
        overlays << overlay;
    }
    return overlays;
}

void layeroverlay::paintEvent(QPaintEvent *){
    QPainter painter(this);
    painter.fillRect(rect(), color);
}
