// SPDX-License-Identifier: LGPL-3.0-or-later

#include "iconbutton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

iconbutton::iconbutton(QIcon icon, int w, int h, bool havetext, QString text){
    setFocusPolicy(Qt::StrongFocus);
    if (havetext == true){
        QVBoxLayout *basevlayout = new QVBoxLayout;
        QHBoxLayout *icohlayout = new QHBoxLayout;
        icohlayout->addStretch(5);
        iconlabel->setPixmap(icon.pixmap(w,h));
        icohlayout->addWidget(iconlabel);
        icohlayout->addStretch(5);
        basevlayout->addLayout(icohlayout);

        QHBoxLayout *texthlayout = new QHBoxLayout;
        texthlayout->addStretch(5);
        textlabel->setText(text);
        texthlayout->addWidget(textlabel);
        texthlayout->addStretch(5);
        basevlayout->addLayout(texthlayout);
        this->setLayout(basevlayout);
    }
    else{
        QVBoxLayout *basevlayout = new QVBoxLayout;
        iconlabel->setPixmap(icon.pixmap(w,h));
        basevlayout->addWidget(iconlabel);
        this->setLayout(basevlayout);
    }
}

void iconbutton::paintEvent(QPaintEvent *){
    QStyleOptionButton option;
    option.initFrom(this);
    if (pressed)
        option.state |= QStyle::State_Sunken;
    else if (focused)
        option.state |= QStyle::State_MouseOver;
    else
        option.state |= QStyle::State_Raised;

    option.state.setFlag(QStyle::State_HasFocus, false);

    QPainter painter(this);
    style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);
}

void iconbutton::focusInEvent(QFocusEvent *){
    focused = true;
    update();
}

void iconbutton::focusOutEvent(QFocusEvent *){
    focused = false;
    update();
}

void iconbutton::mousePressEvent(QMouseEvent *){
    pressed = true;
    update();
}

void iconbutton::mouseReleaseEvent(QMouseEvent *){
    pressed = false;
    update();
    emit clicked();
}

void iconbutton::keyPressEvent(QKeyEvent *event){
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        emit clicked();
    else
        event->ignore();
}
