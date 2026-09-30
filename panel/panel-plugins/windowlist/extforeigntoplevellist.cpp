// SPDX-License-Identifier: LGPL-3.0-or-later

#include "extforeigntoplevellist.h"

#include "extforeigntoplevelhandle.h"

ExtForeignToplevelList::ExtForeignToplevelList(QObject *parent)
    : QWaylandClientExtensionTemplate<ExtForeignToplevelList>(1) {
    setParent(parent);
}

void ExtForeignToplevelList::release() {
    setParent(nullptr);
    if (isActive())
        stop();
    else
        deleteLater();
}

void ExtForeignToplevelList::ext_foreign_toplevel_list_v1_toplevel(struct ::ext_foreign_toplevel_handle_v1 *toplevel) {
    auto *handle = new ExtForeignToplevelHandle(toplevel, this);
    emit toplevelCreated(handle);
}

void ExtForeignToplevelList::ext_foreign_toplevel_list_v1_finished() {
    destroy();
    deleteLater();
}
