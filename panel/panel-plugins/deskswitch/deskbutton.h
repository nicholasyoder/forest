// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKBUTTON_H
#define DESKBUTTON_H

#include <QFrame>
#include <QMouseEvent>

class deskbutton : public QFrame
{
    Q_OBJECT

public:
    deskbutton(int desknumber);

signals:
    void clicked(int num);

public slots:
    void setactive(int num);
    int desknumber(){return desknum;}
    void setNumDeskWindows(int num);

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    void layoutWindows();

    int desknum = 0;
    int numDeskWindows = 0;
    QList<QFrame*> windows;
};

#endif // DESKBUTTON_H
