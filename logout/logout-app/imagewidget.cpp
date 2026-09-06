// SPDX-License-Identifier: LGPL-3.0-or-later

#include "imagewidget.h"

#include <QPainter>

#include <LayerShellQt/Window>

imagewidget::imagewidget(DimLevel level) : level(level){
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);

    winId(); // force native window creation so windowHandle() is valid
    LayerShellQt::Window *layer_window = LayerShellQt::Window::get(windowHandle());
    layer_window->setLayer(LayerShellQt::Window::LayerTop);
    layer_window->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorBottom
                              | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));
    layer_window->setExclusiveZone(-1);
    layer_window->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
    layer_window->setScope("forest-logout-dim");
}

void imagewidget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    painter.fillRect(rect(), level == DimLevel::Full ? QColor(0, 0, 0, 255) : QColor(0, 0, 0, 128));
}
