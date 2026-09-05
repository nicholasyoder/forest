// SPDX-License-Identifier: LGPL-3.0-or-later

#include "extforeigntoplevellist.h"

#include "extforeigntoplevelhandle.h"

ExtForeignToplevelList::ExtForeignToplevelList()
    : QWaylandClientExtensionTemplate<ExtForeignToplevelList>(1) {
}

void ExtForeignToplevelList::ext_foreign_toplevel_list_v1_toplevel(struct ::ext_foreign_toplevel_handle_v1 *toplevel) {
    auto *handle = new ExtForeignToplevelHandle(toplevel);
    emit toplevelCreated(handle);
}
