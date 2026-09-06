// SPDX-License-Identifier: LGPL-3.0-or-later

#include "layeroverlay.h"

#include <QPainter>

layeroverlay::layeroverlay(const QColor &color, LayerShellQt::Window::Layer layer, const QString &scope) : color(color){
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::FramelessWindowHint);

    winId(); // force native window creation so windowHandle() is valid
    LayerShellQt::Window *layer_window = LayerShellQt::Window::get(windowHandle());
    layer_window->setLayer(layer);
    layer_window->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorBottom
                              | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));
    layer_window->setExclusiveZone(-1);
    layer_window->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
    layer_window->setScope(scope);
}

void layeroverlay::paintEvent(QPaintEvent *){
    QPainter painter(this);
    painter.fillRect(rect(), color);
}
