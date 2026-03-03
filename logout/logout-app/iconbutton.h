#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QKeyEvent>
#include <QStyleOptionButton>

class iconbutton : public QFrame{
    Q_OBJECT
public:
    iconbutton(QIcon icon,int w, int h, bool havetext = false, QString text = "");

signals:
    void clicked();

public slots:

private:
    QLabel *iconlabel = new QLabel;
    QLabel *textlabel = new QLabel;

    bool focused = false;
    bool pressed = false;

private slots:
    void paintEvent(QPaintEvent *);
    void enterEvent(QEnterEvent *){this->setFocus();}
    void focusInEvent(QFocusEvent *);
    void leaveEvent(QEvent *){this->clearFocus();}
    void focusOutEvent(QFocusEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *event);
};

#endif // ICONBUTTON_H
