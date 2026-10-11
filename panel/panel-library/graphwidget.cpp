// SPDX-License-Identifier: LGPL-3.0-or-later

#include "graphwidget.h"

#include <QPainter>
#include <QPainterPath>
#include <cmath>

void graphwidget::setupgraphs(int numberofgraphs, QList<QColor> gcolors, QList<qreal> gopacitys, QColor gbackcolor, qreal gbackopacity)
{
    samples = QList<QList<qreal>>(numberofgraphs);
    colors = gcolors;
    opacitys = gopacitys;
    backcolor = gbackcolor;
    backopacity = gbackopacity;
    update();
}

void graphwidget::updategraph(QList<qreal> newvalues)
{
    if (newvalues.count() != samples.count()) return;

    for (int i = 0; i < samples.count(); i++) {
        qreal v = newvalues[i];
        samples[i].append(std::isfinite(v) ? qBound(0.0, v, 1.0) : 0.0);
    }
    trim();
    update();
}

void graphwidget::trim()
{
    for (QList<qreal> &s : samples)
        if (s.count() > width())
            s.remove(0, s.count() - width());
}

void graphwidget::resizeEvent(QResizeEvent *)
{
    trim();
}

void graphwidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setOpacity(backopacity);
    painter.fillRect(rect(), backcolor);
    painter.setRenderHint(QPainter::Antialiasing);

    const qreal w = width(), h = height();
    for (int g = 0; g < samples.count(); g++) {
        const QList<qreal> &s = samples[g];
        const int n = s.count();
        if (n == 0) continue;

        // Polyline through sample centres, not steps: vertical edges would land
        // on half device pixels at 1.5x. Ends extend flat to the widget edges.
        const qreal x0 = w - n;
        QPainterPath path(QPointF(x0, h));
        path.lineTo(x0, h * (1 - s[0]));
        for (int i = 0; i < n; i++)
            path.lineTo(x0 + i + 0.5, h * (1 - s[i]));
        path.lineTo(w, h * (1 - s[n - 1]));
        path.lineTo(w, h);
        path.closeSubpath();

        painter.setOpacity(opacitys.value(g, 1));
        painter.fillPath(path, colors.value(g));
    }
}
