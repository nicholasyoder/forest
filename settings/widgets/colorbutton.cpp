// SPDX-License-Identifier: LGPL-3.0-or-later

#include "colorbutton.h"

#include <QColorDialog>
#include <QPainter>
#include <QStyleOptionButton>

ColorButton::ColorButton(QWidget *parent) : QPushButton(parent){
    setObjectName("ColorButton");
    setMinimumWidth(56);
    connect(this, &QPushButton::clicked, this, &ColorButton::pick);
}

void ColorButton::setColor(const QColor &color){
    if (color == current) return;
    current = color;
    setToolTip(color.name(QColor::HexArgb));
    update();
    emit colorChanged(color);
}

void ColorButton::pick(){
    QColor color = QColorDialog::getColor(current, window(), tr("Choose Color"), QColorDialog::ShowAlphaChannel);
    if (color.isValid()) setColor(color);
}

void ColorButton::paintEvent(QPaintEvent *event){
    QPushButton::paintEvent(event);
    QStyleOptionButton option;
    initStyleOption(&option);
    QRect swatch = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this).adjusted(2, 2, -2, -2);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    // Checkerboard under the color so alpha is visible.
    painter.setClipRect(swatch);
    const int cell = 4;
    for (int y = swatch.top(); y < swatch.bottom(); y += cell)
        for (int x = swatch.left(); x < swatch.right(); x += cell)
            painter.fillRect(x, y, cell, cell, ((x - swatch.left()) / cell + (y - swatch.top()) / cell) % 2 ? Qt::lightGray : Qt::white);
    painter.fillRect(swatch, current);
    painter.setClipping(false);
    painter.setPen(QColor(0, 0, 0, 80));
    painter.drawRect(QRectF(swatch).adjusted(0.5, 0.5, -0.5, -0.5));
}
