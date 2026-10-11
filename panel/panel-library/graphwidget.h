// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>

class graphwidget : public QWidget
{
    Q_OBJECT

public slots:
    void setupgraphs(int numberofgraphs, QList<QColor> gcolors, QList<qreal> gopacitys, QColor gbackcolor, qreal gbackopacity);
    void updategraph(QList<qreal> newvalues);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:
    void trim();

    // Per graph, oldest first, one sample per logical pixel of width.
    QList<QList<qreal>> samples;
    QList<QColor> colors;
    QList<qreal> opacitys;
    QColor backcolor;
    qreal backopacity = 1;
};

#endif // GRAPHWIDGET_H
