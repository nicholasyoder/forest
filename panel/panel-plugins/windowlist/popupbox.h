// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef POPUPBOX_H
#define POPUPBOX_H

#include <QWidget>
#include <QTimer>
#include "button.h"

class popupbox : public QWidget
{
    Q_OBJECT

public:
    popupbox(unsigned long window);
    ~popupbox();

    void showanim();

signals:
    //void clicked();
    void raisewin(unsigned long window);
    void maxwin(unsigned long window);
    void minwin(unsigned long window);
    void closewin(unsigned long window);

private slots:
    void closepop(unsigned long window);
    void incrementanim();

    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);

private:

    QTimer *animtimer = new QTimer;
    int animfactor = 0;

};

#endif // POPUPBOX_H
