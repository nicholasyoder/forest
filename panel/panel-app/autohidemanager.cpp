// SPDX-License-Identifier: LGPL-3.0-or-later

#include "autohidemanager.h"

#include <QEvent>
#include <QGuiApplication>
#include <QWidget>

void kill_timer(QTimer*& timer){
    if(timer){
        timer->stop();
        delete timer;
        timer = nullptr;
    }
}

void start_timer(QTimer*& timer, AutoHideManager* obj, void (AutoHideManager::*func)()){
    kill_timer(timer);
    timer = new QTimer(obj);
    AutoHideManager::connect(timer, &QTimer::timeout, obj, func);
    timer->start(500);
}

AutoHideManager::AutoHideManager(QObject *parent) : QObject{parent}{}

AutoHideManager::~AutoHideManager(){
    kill_timer(check_focus_timer);
    kill_timer(maybe_close_timer);
}

bool AutoHideManager::eventFilter(QObject* obj, QEvent* event){
    if (event->type() == QEvent::Show) {
        QWidget* widget = qobject_cast<QWidget*>(obj);
        if(widget)
            widget->window()->activateWindow();
        close_eventually();
        return true;
    }
    return QObject::eventFilter(obj, event);
}

void AutoHideManager::close_eventually(){
    start_timer(check_focus_timer, this, &AutoHideManager::check_focus);
}

void AutoHideManager::check_focus(){
    if(QGuiApplication::focusWindow() == nullptr){
        lost_focus_time = QDateTime::currentDateTime();
        kill_timer(check_focus_timer);
        start_timer(maybe_close_timer, this, &AutoHideManager::maybe_close);
    }
}

void AutoHideManager::maybe_close(){
    if(QGuiApplication::focusWindow() == nullptr){
        if(lost_focus_time.secsTo(QDateTime::currentDateTime()) > 2){
            emit hide();
            kill_timer(maybe_close_timer);
        }
    }
    else {
        kill_timer(maybe_close_timer);
        close_eventually();
    }
}
