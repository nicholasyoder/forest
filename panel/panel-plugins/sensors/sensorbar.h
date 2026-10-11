// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SENSORBAR_H
#define SENSORBAR_H

#include <QFrame>
#include <QColor>

// One temperature bar; QSS styles the frame, the fill is painted over contentsRect().
class SensorBar : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QColor lowColor MEMBER lowColor)
    Q_PROPERTY(QColor midColor MEMBER midColor)
    Q_PROPERTY(QColor highColor MEMBER highColor)

public:
    explicit SensorBar(Qt::Orientation orientation, QWidget *parent = nullptr);
    void setValue(qreal fraction); // 0–1 of maxtemp

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Qt::Orientation orientation;
    qreal value = 0;
    QColor lowColor = Qt::green;
    QColor midColor = Qt::yellow;
    QColor highColor = Qt::red;
};

#endif // SENSORBAR_H
