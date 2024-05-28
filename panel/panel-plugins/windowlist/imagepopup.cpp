#include "imagepopup.h"
#include <QGraphicsDropShadowEffect>

imagepopup::imagepopup(QWidget *parentw){
    parentwidget = parentw;

    popupglayout = new QGridLayout;
    popupglayout->setMargin(0);
    popupglayout->setSpacing(3);

    closebutton *cbt = new closebutton;
    cbt->setFixedSize(18,18);
    popupglayout->addWidget(cbt, 0,1);

    QGraphicsDropShadowEffect *ds_effect = new QGraphicsDropShadowEffect();
    ds_effect->setBlurRadius(18);
    ds_effect->setOffset(0,0);
    ds_effect->setColor(Qt::black);
    scrshotlabel = new QLabel;
    scrshotlabel->setStyleSheet("margin: 10px;");
    scrshotlabel->setGraphicsEffect(ds_effect);
    popupglayout->addWidget(scrshotlabel, 1,0, 1, 2);
    pbox = new popup(popupglayout, parentwidget, CenteredOnWidget);
    pbox->setWindowFlag(Qt::Popup, false);
    pbox->setWindowFlag(Qt::X11BypassWindowManagerHint, true);
    connect(cbt, &closebutton::clicked, pbox, &popup::closepopup);
    connect(cbt, &closebutton::clicked, this, &imagepopup::closewindow);

    connect(pbox, &popup::mousereleased, this, &imagepopup::raisewindow);
}

void imagepopup::btmouseEnter(windowbutton *bt){
    if (!popup_enabled) return;

    currentbt = bt;

    if (!open){
        if (openptimer)
            delete openptimer;

        openptimer = new QTimer;
        openptimer->setSingleShot(true);
        connect(openptimer, &QTimer::timeout, this, &imagepopup::showpopup);
        openptimer->start(450);
    }
    else {
        QPixmap pix = get_window_image();

        scrshotlabel->setPixmap(QPixmap());
        pbox->setMinimumSize(0,0);
        pbox->resize(10,10);
        pbox->close();

        scrshotlabel->resize(pix.size());
        scrshotlabel->setPixmap(pix);
        pbox->changelauncher(currentbt);
        pbox->positionOnLauncher();
        pbox->show();
    }
}

void imagepopup::btmouseLeave(){
    if (!popup_enabled) return;
    deleteopenptimer();
}

void imagepopup::btclicked(){
    if (!currentbt) return;

    if (pbox->isVisible())
        closepopup();
    else
        deleteopenptimer();
}

void imagepopup::showpopup(){
    if (!currentbt) return;

    QPixmap pix = get_window_image();

    scrshotlabel->resize(pix.size());
    scrshotlabel->setPixmap(pix);
    pbox->changelauncher(currentbt);
    pbox->showpopup();

    open = true;

    if (closeptimer){
        closeptimer->stop();
        delete closeptimer;
    }
    closeptimer = new QTimer;
    connect(closeptimer, &QTimer::timeout, this, &imagepopup::tryclosepopup);
    closeptimer->start(450);
}

void imagepopup::tryclosepopup(){
    if (!currentbt){
        if (closeptimer)
        {
            closeptimer->stop();
            delete closeptimer;
            closeptimer = nullptr;
        }
        return;
    }

    QRect r(0,0, currentbt->width(), currentbt->height());

    if (!(r.contains(currentbt->mapFromGlobal(QCursor::pos())) || pbox->geometry().contains(QCursor::pos())))
    {
        closepopup();
    }
}

void imagepopup::closepopup(){
    if (!popup_enabled) return;
    open = false;

    if (closeptimer)
    {
        closeptimer->stop();
        delete closeptimer;
        closeptimer = nullptr;
    }

    currentbt = nullptr;

    scrshotlabel->setPixmap(QPixmap());
    pbox->setMinimumSize(0,0);
    pbox->resize(10,10);
    pbox->closepopup();
}

void imagepopup::deleteopenptimer(){
    if (openptimer){
        openptimer->stop();
        delete openptimer;
        openptimer = nullptr;
    }
}

QPixmap imagepopup::get_window_image(){
    QPixmap pix = QPixmap::fromImage(Xcbutills::getWindowImage(currentbt->windowId()));
    if (pix.height() > 120) pix = pix.scaledToHeight(120, Qt::SmoothTransformation);
    return pix;
}
