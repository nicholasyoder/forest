// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LAYEROVERLAY_H
#define LAYEROVERLAY_H

#include <QWidget>
#include <QColor>

#include <LayerShellQt/Window>

class QScreen;

class layeroverlay : public QWidget{
    Q_OBJECT
public:
    layeroverlay(const QColor &color, LayerShellQt::Window::Layer layer, const QString &scope, QScreen *screen);
    static QList<layeroverlay*> showOnAllScreens(const QColor &color, LayerShellQt::Window::Layer layer, const QString &scope);

private:
    void paintEvent(QPaintEvent *);

    QColor color;
};

#endif // LAYEROVERLAY_H
