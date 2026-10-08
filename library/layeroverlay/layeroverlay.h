// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LAYEROVERLAY_H
#define LAYEROVERLAY_H

#include <QWidget>
#include <QColor>
#include <QPointer>

#include <LayerShellQt/Window>

class QScreen;

class layeroverlay : public QWidget{
    Q_OBJECT
public:
    // passInput: empty input region, so clicks reach whatever is below.
    layeroverlay(const QColor &color, LayerShellQt::Window::Layer layer, const QString &scope, QScreen *screen, bool passInput = false);
    // QPointers: an overlay deletes itself when its output goes away.
    static QList<QPointer<layeroverlay>> showOnAllScreens(const QColor &color, LayerShellQt::Window::Layer layer, const QString &scope, bool passInput = false);

private:
    void paintEvent(QPaintEvent *);

    QColor color;
};

#endif // LAYEROVERLAY_H
