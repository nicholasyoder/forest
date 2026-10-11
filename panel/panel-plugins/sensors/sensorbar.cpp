// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sensorbar.h"

#include <QPainter>

SensorBar::SensorBar(Qt::Orientation orientation, QWidget *parent) : QFrame(parent), orientation(orientation){
    setObjectName("sensorBar");
}

void SensorBar::setValue(qreal fraction){
    fraction = qBound(0.0, fraction, 1.0);
    if (fraction == value) return;
    value = fraction;
    update();
}

void SensorBar::paintEvent(QPaintEvent *event){
    QFrame::paintEvent(event);

    // Gradient spans the whole bar, not just the fill, so hotter bars reach redder.
    QRectF r = contentsRect();
    QLinearGradient gradient;
    QRectF fill = r;
    if (orientation == Qt::Vertical){
        gradient = QLinearGradient(r.bottomLeft(), r.topLeft());
        fill.setTop(r.bottom() - r.height() * value);
    }
    else{
        gradient = QLinearGradient(r.topLeft(), r.topRight());
        fill.setWidth(r.width() * value);
    }
    gradient.setColorAt(0, lowColor);
    gradient.setColorAt(0.5, midColor);
    gradient.setColorAt(0.8, highColor);

    QPainter painter(this);
    painter.fillRect(fill, gradient);
}
