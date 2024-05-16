#ifndef BUTTONS_H
#define BUTTONS_H

#include <QColor>
#include <QImage>

class buttons
{
    Q_OBJECT

public:
    buttons(){}
    ~buttons(){}

    static QString highlighttype;
    static QImage highlightimage;
    static QImage pressedimage;
    static QColor pressedcolor;
    static QColor highlightcolor;
    static qreal opacity;
};

#endif // BUTTONS_H
