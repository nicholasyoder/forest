#include "desktopicon.h"

desktopicon::desktopicon(QString text, QIcon ico, QString ID, QMenu *contextmenu){
    icon = ico;
    iconID = ID;
    cmenu = contextmenu;

    //setup widget
    this->setFixedSize(96, 96);
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setMargin(0);
    vlayout->addSpacing(46);

    setToolTip(text);
    tlabel = new elidedlabel(text);
    tlabel->setForegroundRole(QPalette::Light);
    stkwidget->addWidget(tlabel);
    stkwidget->addWidget(renametBox);
    connect(renametBox, &QLineEdit::editingFinished, this, &desktopicon::handleTextEditFinished);
    vlayout->addWidget(stkwidget, 1);
    this->setLayout(vlayout);

    setObjectName("desktopIcon");
}

void desktopicon::settext(QString text){
    tlabel->setText(text);
    shadowlabel->setText(text);
}

void desktopicon::canceldrag(){
    move(posB4drag);
}

void desktopicon::paintEvent(QPaintEvent *){
    QStyleOptionButton option;
    option.initFrom(this);
    if (selected)
        option.state |= QStyle::State_MouseOver;
    else
        option.state |= QStyle::State_Raised;

    QPainter painter(this);
    style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);

    int x = this->width() / 2 - 24;
    int y = 2;
    painter.drawPixmap(x, y, 48, 48, this->icon.pixmap(QSize(48,48), QIcon::Normal,QIcon::On));
}

void desktopicon::mousePressEvent(QMouseEvent *event){
    posB4drag = this->pos();
    dragpos = event->globalPos() - frameGeometry().topLeft();

    if (selected == false || event->button() == Qt::LeftButton){
        selected = true;
        this->update();
        emit sigselected(iconID);
        raise();
    }

    delete dragtimer;
    dragtimer = new QTimer;
    dragtimer->setSingleShot(true);
    connect(dragtimer, &QTimer::timeout, this, &desktopicon::setallowdrag);
    dragtimer->start(100);
}

void desktopicon::mouseMoveEvent(QMouseEvent *event){
    QFrame::mouseMoveEvent(event);
    if (allowdrag == true){
        this->move(event->globalPos() - dragpos);
        dragged = true;
    }
}

void desktopicon::mouseReleaseEvent(QMouseEvent *event){
    dragtimer->stop();

    if (dragged == true){
        dragged = false;
        allowdrag = false;

        emit sigdragged(this);
    }
    else{
        allowdrag = false;
    }

    if (event->button() == Qt::LeftButton){
        if (ondoubleclick == true){
            emit sigactivated(iconID);
        }
        else{
            ondoubleclick = true;
            QTimer::singleShot(300,this, &desktopicon::stopcurrentdoubleclick);
        }
    }
    else if (event->button() == Qt::RightButton){
        cmenu->exec(event->globalPos());
    }
}

void desktopicon::handleTextEditFinished(){
    stkwidget->setCurrentIndex(0);
    emit textchanged(this->getID(), renametBox->text());
}
