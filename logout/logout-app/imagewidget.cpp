#include "imagewidget.h"

imagewidget::imagewidget(){
    setWindowOpacity(0.0);
    setWindowFlags(Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void imagewidget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    painter.setOpacity(opacity);
    painter.fillRect(0,0,width(),height(), QColor(0,0,0,255));
}

void imagewidget::start(){
    this->setWindowOpacity(1.0);
    connect(animtimer, SIGNAL(timeout()), this, SLOT(fade()));
    animstarttime = QDateTime::currentMSecsSinceEpoch();
    animtime = 100;
    animtimer->start(0);
}

void imagewidget::blackout(){
    delete t;
    t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SLOT(fadeblack()));
    t->start(0);
}

void imagewidget::stop(){
    delete t;
    t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SLOT(fadeout()));
    t->start(0);
}

void imagewidget::fade(){
    qint64 timepassed = QDateTime::currentMSecsSinceEpoch() - animstarttime;

    if (timepassed >= animtime){
        animtimer->stop();
    }
    else{
        opacity = (qreal(timepassed) / qreal(animtime)) / 2;
        update();
    }
}

void imagewidget::fadeblack(){
    if (opacity < 1.0){
        opacity = opacity + 0.05;
        update();
    }
    else{
        t->stop();
    }
}

void imagewidget::fadeout(){
    if (opacity > 0.0){
        opacity = opacity - 0.05;
        update();
    }
    else{
        t->stop();
        this->close();
    }
}
