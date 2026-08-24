// SPDX-License-Identifier: LGPL-3.0-or-later

#include "foreigntoplevelmanager.h"

#include "foreigntoplevelhandle.h"

ForeignToplevelManager::ForeignToplevelManager()
    : QWaylandClientExtensionTemplate<ForeignToplevelManager>(1){
}

void ForeignToplevelManager::zwlr_foreign_toplevel_manager_v1_toplevel(struct ::zwlr_foreign_toplevel_handle_v1 *toplevel){
    ForeignToplevelHandle *handle = new ForeignToplevelHandle(toplevel);
    emit toplevelCreated(handle);
}
