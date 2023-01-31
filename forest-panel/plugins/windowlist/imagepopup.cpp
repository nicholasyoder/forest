#include "imagepopup.h"

imagepopup::imagepopup(QWidget *parentw)
{
    parentwidget = parentw;

    popupglayout = new QGridLayout;
    popupglayout->setMargin(0);
    popupglayout->setSpacing(3);

    closebutton *cbt = new closebutton;
    cbt->setFixedSize(18,18);
    popupglayout->addWidget(cbt, 0,1);

    scrshotlabel = new QLabel;
    popupglayout->addWidget(scrshotlabel, 1,0, 1, 2);
    pbox = new popup(popupglayout, parentwidget, CenteredOnWidget);
    pbox->setWindowFlag(Qt::Popup, false);
    pbox->setWindowFlag(Qt::X11BypassWindowManagerHint, true);
    connect(cbt, &closebutton::clicked, pbox, &popup::closepopup);
    connect(cbt, &closebutton::clicked, this, &imagepopup::closewindow);

    connect(pbox, &popup::mousereleased, this, &imagepopup::raisewindow);
}

void imagepopup::btmouseEnter(windowbutton *bt)
{
    currentbt = bt;

    if (!open){
        if (openptimer)
            delete openptimer;

        openptimer = new QTimer;
        openptimer->setSingleShot(true);
        connect(openptimer, &QTimer::timeout, this, &imagepopup::showpopup);
        openptimer->start(500);
    }
    else {
        QPixmap pix = QPixmap::fromImage(Xcbutills::getWindowImage(currentbt->windowId())).scaledToHeight(100, Qt::SmoothTransformation);

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

void imagepopup::btmouseLeave()
{
    deleteopenptimer();
}

void imagepopup::btclicked()
{
    if (!currentbt) return;

    if (pbox->isVisible())
        closepopup();
    else
        deleteopenptimer();
}

void imagepopup::showpopup()
{
    if (!currentbt) return;

    QPixmap pix = QPixmap::fromImage(Xcbutills::getWindowImage(currentbt->windowId())).scaledToHeight(100, Qt::SmoothTransformation);

    scrshotlabel->resize(pix.size());
    scrshotlabel->setPixmap(pix);
    pbox->changelauncher(currentbt);
    pbox->showpopup();

    open = true;

    closeptimer = new QTimer;
    connect(closeptimer, &QTimer::timeout, this, &imagepopup::tryclosepopup);
    closeptimer->start(200);
}

void imagepopup::tryclosepopup()
{
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

void imagepopup::closepopup()
{
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

void imagepopup::deleteopenptimer()
{
    if (openptimer){
        openptimer->stop();
        delete openptimer;
        openptimer = nullptr;
    }
}
