// SPDX-License-Identifier: LGPL-3.0-or-later

#include "foreigntoplevelhandle.h"

#include <QGuiApplication>
#include <qguiapplication_platform.h>

ForeignToplevelHandle::ForeignToplevelHandle(struct ::zwlr_foreign_toplevel_handle_v1 *object)
    : QtWayland::zwlr_foreign_toplevel_handle_v1(object){
}

ForeignToplevelHandle::~ForeignToplevelHandle(){
    destroy();
}

void ForeignToplevelHandle::activate(){
    auto *waylandApp = qApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (waylandApp)
        QtWayland::zwlr_foreign_toplevel_handle_v1::activate(waylandApp->seat());
}

void ForeignToplevelHandle::setMaximized(){
    QtWayland::zwlr_foreign_toplevel_handle_v1::set_maximized();
}

void ForeignToplevelHandle::unsetMaximized(){
    QtWayland::zwlr_foreign_toplevel_handle_v1::unset_maximized();
}

void ForeignToplevelHandle::setMinimized(){
    QtWayland::zwlr_foreign_toplevel_handle_v1::set_minimized();
}

void ForeignToplevelHandle::requestClose(){
    QtWayland::zwlr_foreign_toplevel_handle_v1::close();
}

void ForeignToplevelHandle::zwlr_foreign_toplevel_handle_v1_title(const QString &title){
    m_title = title;
}

void ForeignToplevelHandle::zwlr_foreign_toplevel_handle_v1_app_id(const QString &app_id){
    m_appId = app_id;
}

void ForeignToplevelHandle::zwlr_foreign_toplevel_handle_v1_state(wl_array *state){
    m_maximized = false;
    m_minimized = false;
    m_activated = false;

    // wl_array_for_each() assigns (array)->data (void*) straight into pos,
    // which is a C-only implicit conversion - not valid in C++ - so this
    // walks the array by hand instead.
    const uint32_t *entries = static_cast<const uint32_t *>(state->data);
    size_t count = state->size / sizeof(uint32_t);
    for (size_t i = 0; i < count; i++){
        switch (entries[i]){
        case state_maximized: m_maximized = true; break;
        case state_minimized: m_minimized = true; break;
        case state_activated: m_activated = true; break;
        default: break;
        }
    }
}

void ForeignToplevelHandle::zwlr_foreign_toplevel_handle_v1_done(){
    emit changed(this);
}

void ForeignToplevelHandle::zwlr_foreign_toplevel_handle_v1_closed(){
    emit closed(this);
}
