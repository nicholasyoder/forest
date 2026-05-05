// SPDX-License-Identifier: LGPL-3.0-or-later

#include "hiddenpanel.h"

HiddenPanel::HiddenPanel(QWidget *parent) : QWidget{parent}{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    setWindowFlags(Qt::FramelessWindowHint);
    setMouseTracking(true);
}

void HiddenPanel::mouseMoveEvent(QMouseEvent *){
    hide();
    emit activated();
}
