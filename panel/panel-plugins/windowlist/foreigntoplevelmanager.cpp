// SPDX-License-Identifier: LGPL-3.0-or-later

#include "foreigntoplevelmanager.h"

#include "foreigntoplevelhandle.h"

ForeignToplevelManager::ForeignToplevelManager(QObject *parent)
    : QWaylandClientExtensionTemplate<ForeignToplevelManager>(1){
    setParent(parent);
}

void ForeignToplevelManager::release(){
    setParent(nullptr);
    if (isActive())
        stop();
    else
        deleteLater();
}

void ForeignToplevelManager::zwlr_foreign_toplevel_manager_v1_toplevel(struct ::zwlr_foreign_toplevel_handle_v1 *toplevel){
    ForeignToplevelHandle *handle = new ForeignToplevelHandle(toplevel, this);
    emit toplevelCreated(handle);
}

void ForeignToplevelManager::zwlr_foreign_toplevel_manager_v1_finished(){
    // No destructor request in this protocol; the proxy is destroyed client-side only.
    wl_proxy_destroy(reinterpret_cast<wl_proxy *>(object()));
    deleteLater();
}
