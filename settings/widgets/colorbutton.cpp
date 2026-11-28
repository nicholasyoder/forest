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

void ColorButton::setAlphaEnabled(bool enabled){
    if (alpha_enabled == enabled) return;
    alpha_enabled = enabled;
    if (!alpha_enabled && current.alpha() != 255)
        setColor(QColor(current.red(), current.green(), current.blue()));
    else
        setToolTip(current.name(alpha_enabled ? QColor::HexArgb : QColor::HexRgb));
    update();
}

void ColorButton::setColor(const QColor &color){
    QColor normalized = color;
    if (!alpha_enabled && normalized.isValid())
        normalized.setAlpha(255);
    if (normalized == current) return;
    current = normalized;
    setToolTip(current.name(alpha_enabled ? QColor::HexArgb : QColor::HexRgb));
    update();
    emit colorChanged(current);
}

void ColorButton::pick(){
    QColorDialog::ColorDialogOptions opts;
    if (alpha_enabled)
        opts |= QColorDialog::ShowAlphaChannel;
    QColor color = QColorDialog::getColor(current, window(), tr("Choose Color"), opts);
    if (color.isValid()) setColor(color);
}

void ColorButton::paintEvent(QPaintEvent *event){
    QPushButton::paintEvent(event);
    QStyleOptionButton option;
    initStyleOption(&option);
    QRect swatch = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this).adjusted(2, 2, -2, -2);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(swatch);
    if (alpha_enabled) {
        // Checkerboard under the color so alpha is visible.
        const int cell = 4;
        for (int y = swatch.top(); y < swatch.bottom(); y += cell)
            for (int x = swatch.left(); x < swatch.right(); x += cell)
                painter.fillRect(x, y, cell, cell, ((x - swatch.left()) / cell + (y - swatch.top()) / cell) % 2 ? Qt::lightGray : Qt::white);
    }
    painter.fillRect(swatch, current);
    painter.setClipping(false);
    painter.setPen(QColor(0, 0, 0, 80));
    painter.drawRect(QRectF(swatch).adjusted(0.5, 0.5, -0.5, -0.5));
}
