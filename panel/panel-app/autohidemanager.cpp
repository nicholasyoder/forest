// SPDX-License-Identifier: LGPL-3.0-or-later

#include "autohidemanager.h"

#include <QEvent>
#include <QGuiApplication>
#include <QWindow>

#include "geometrymanager.h"

AutoHideManager::AutoHideManager(GeometryManager *geometry, QObject *parent)
    : QObject{parent}, geometry(geometry) {
    hide_timer.setSingleShot(true);
    hide_timer.setInterval(1000);
    connect(&hide_timer, &QTimer::timeout, this, &AutoHideManager::maybe_hide);
    connect(qApp, &QGuiApplication::focusWindowChanged, this, &AutoHideManager::handle_focus_change);
    connect(geometry, &GeometryManager::shell_changed, this, &AutoHideManager::watch_shell);
    watch_shell(geometry->shell_widget());
}

AutoHideManager::~AutoHideManager(){
    if (shell)
        shell->removeEventFilter(this);
    geometry->set_collapsed(false);
}

void AutoHideManager::set_delay(int ms){
    hide_timer.setInterval(ms);
}

void AutoHideManager::start(){
    hide_timer.start();
}

void AutoHideManager::watch_shell(QWidget *new_shell){
    if (shell)
        shell->removeEventFilter(this);
    shell = new_shell;
    shell->installEventFilter(this);
}

bool AutoHideManager::eventFilter(QObject* obj, QEvent* event){
    if (obj == shell) {
        if (event->type() == QEvent::Enter) {
            hide_timer.stop();
            geometry->set_collapsed(false);
        } else if (event->type() == QEvent::Leave) {
            hide_timer.start();
        }
    }
    return QObject::eventFilter(obj, event);
}

// Reveals for popups opened while collapsed (e.g. main menu hotkey); the
// timer then polls until the popup closes, however it's closed.
void AutoHideManager::handle_focus_change(QWindow *focus){
    if (is_panel_popup(focus))
        geometry->set_collapsed(false);
    hide_timer.start();
}

void AutoHideManager::maybe_hide(){
    if (!shell || shell->underMouse())
        return; // Leave re-arms
    if (panel_popup_visible()) {
        hide_timer.start(); // popup close gives no reliable signal here
        return;
    }
    geometry->set_collapsed(true);
}

// A window transient to the shell (panel-library/popup.h sets that parent).
bool AutoHideManager::is_panel_popup(QWindow *window) const {
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
