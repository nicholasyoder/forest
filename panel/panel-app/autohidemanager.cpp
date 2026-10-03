// SPDX-License-Identifier: LGPL-3.0-or-later

#include "autohidemanager.h"

#include <QEvent>
#include <QGuiApplication>
#include <QWidget>
#include <QWindow>

#include "geometrymanager.h"

// App-wide filter: QWindow sends Show/Hide via sendEvent, so popups are seen regardless of focus.
AutoHideManager::AutoHideManager(GeometryManager *geometry, QObject *parent)
    : QObject{parent}, geometry(geometry) {
    hide_timer.setSingleShot(true);
    hide_timer.setInterval(1000);
    connect(&hide_timer, &QTimer::timeout, this, &AutoHideManager::maybe_hide);
    qApp->installEventFilter(this);
}

AutoHideManager::~AutoHideManager(){
    qApp->removeEventFilter(this);
    geometry->set_collapsed(false);
}

void AutoHideManager::set_delay(int ms){
    hide_timer.setInterval(ms);
}

void AutoHideManager::start(){
    hide_timer.start();
}

bool AutoHideManager::eventFilter(QObject* obj, QEvent* event){
    switch (event->type()) {
    case QEvent::Enter:
        if (obj == geometry->shell_widget()) {
            hide_timer.stop();
            geometry->set_collapsed(false);
        }
        break;
    case QEvent::Leave:
        if (obj == geometry->shell_widget())
            hide_timer.start();
        break;
    case QEvent::Show: // reveals for popups opened while collapsed (e.g. main menu hotkey)
        if (obj->isWindowType() && is_panel_popup(static_cast<QWindow*>(obj))) {
            hide_timer.stop();
            geometry->set_collapsed(false);
        }
        break;
    case QEvent::Hide:
        if (obj->isWindowType() && is_panel_popup(static_cast<QWindow*>(obj)))
            hide_timer.start();
        break;
    default:
        break;
    }
    return QObject::eventFilter(obj, event);
}

// No re-arm: the next shell Leave or popup Hide restarts the timer.
void AutoHideManager::maybe_hide(){
    QWidget *shell = geometry->shell_widget();
    if (!shell || shell->underMouse() || panel_popup_visible())
        return;
    geometry->set_collapsed(true);
}

// A window transient to the shell (panel-library/popup.h sets that parent).
bool AutoHideManager::is_panel_popup(QWindow *window) const {
    QWidget *shell = geometry->shell_widget();
    if (!shell || !window)
        return false;
    QWindow *shell_window = shell->windowHandle();
    for (window = window->transientParent(); window; window = window->transientParent()) {
        if (window == shell_window)
            return true;
    }
    return false;
}

bool AutoHideManager::panel_popup_visible() const {
    for (QWindow *window : QGuiApplication::topLevelWindows()) {
        if (window->isVisible() && is_panel_popup(window))
            return true;
    }
    return false;
}
