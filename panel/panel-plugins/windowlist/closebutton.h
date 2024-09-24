#ifndef CLOSEBUTTON_H
#define CLOSEBUTTON_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

class closebutton : public QWidget
{
    Q_OBJECT

public:
    closebutton(){}

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
        QPen pen;
        pen.setWidth(2);

        if (pressed)
        {
            pen.setColor(QColor::fromRgb(150,0,0));
        }
        else if (mouseover)
        {
            pen.setColor(QColor::fromRgb(200,0,0));
        }
        else
        {
            pen.setColor(QColor::fromRgb(125,125,125));
        }

        painter.setPen(pen);
        painter.drawLine(4,4,this->width()-5, this->height()-5);
        painter.drawLine(this->width()-5,4, 4, this->height()-5);
    }

    void enterEvent(QEvent *)
    {
        mouseover = true;
        update();
    }

    void leaveEvent(QEvent *)
    {
        mouseover = false;
        update();
    }

    void mousePressEvent(QMouseEvent *)
    {
        pressed = true;
        update();
    }

    void mouseReleaseEvent(QMouseEvent *event)
    {
        pressed = false;
        update();

        if (event->button() == Qt::LeftButton)
            emit clicked();
    }

private:
    bool mouseover = false;
    bool pressed = false;
};

#endif // CLOSEBUTTON_H
