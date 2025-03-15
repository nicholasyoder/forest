#ifndef FADEWIDGET_H
#define FADEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>

#include "miscutills.h"

class fadewidget : public QWidget{
    Q_OBJECT

public:
    fadewidget(QScreen *screen);

    qreal opacity = 0.0;
    QImage *backimage = new QImage();
    QImage *wallpaper = new QImage();
    QTimer *t = new QTimer;

public slots:
    void getimage();
    void start();

protected:
    void paintEvent(QPaintEvent *);

private slots:
    void fadein();

private:
    WALLPAPER_MODE imagemode;
    QScreen *wscreen;
};

#endif // FADEWIDGET_H
