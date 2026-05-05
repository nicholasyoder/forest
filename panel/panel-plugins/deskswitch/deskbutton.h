// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKBUTTON_H
#define DESKBUTTON_H

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>

class deskbutton : public QWidget
{
    Q_OBJECT

public:
    deskbutton(int desknumber);

signals:
    void clicked(int num);

public slots:
    void setactive(int num);
    int desknumber(){return desknum;}
    void setNumDeskWindows(int num){numDeskWindows = num; update();}

protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    int desknum = 0;
    bool active = false;
    int numDeskWindows=0;

};

#endif // DESKBUTTON_H
