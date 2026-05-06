// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QDir>
#include <QDateTime>

class imagewidget : public QWidget{
    Q_OBJECT
public:
    imagewidget();

public slots:
    void start();
    void blackout();
    void stop();

private slots:
    void paintEvent(QPaintEvent *);
    void fade();
    void fadeblack();
    void fadeout();

private:
    qint64 animstarttime = 0;
    qint64 animtime = 0;
    QTimer *animtimer = new QTimer;
    qreal opacity = 0.0;
    QTimer *t = new QTimer;
};

#endif // IMAGEWIDGET_H
