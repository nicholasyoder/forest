// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LAYEROVERLAY_H
#define LAYEROVERLAY_H

#include <QColor>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>

#include <memory>

class QScreen;

// Full-output solid-color layer surface. Painted with a single-pixel buffer
// scaled by wp_viewport, so no output-sized buffer is ever uploaded.
class layeroverlay : public QObject{
    Q_OBJECT
public:
    enum class Layer { Background, Bottom, Top, Overlay };

    // passInput: empty input region, so clicks reach whatever is below.
    layeroverlay(const QColor &color, Layer layer, const QString &scope, QScreen *screen, bool passInput = false);
    ~layeroverlay() override;

    // QPointers: an overlay deletes itself when its output goes away.
    static QList<QPointer<layeroverlay>> showOnAllScreens(const QColor &color, Layer layer, const QString &scope, bool passInput = false);

public slots:
    // Unmaps immediately, then deletes the object.
    void close();

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

#endif // LAYEROVERLAY_H
