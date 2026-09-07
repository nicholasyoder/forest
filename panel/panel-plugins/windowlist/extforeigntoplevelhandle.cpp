// SPDX-License-Identifier: LGPL-3.0-or-later

#include "extforeigntoplevelhandle.h"

ExtForeignToplevelHandle::ExtForeignToplevelHandle(struct ::ext_foreign_toplevel_handle_v1 *object)
    : QtWayland::ext_foreign_toplevel_handle_v1(object) {
}

ExtForeignToplevelHandle::~ExtForeignToplevelHandle() {
    destroy();
}

void ExtForeignToplevelHandle::ext_foreign_toplevel_handle_v1_identifier(const QString &identifier) {
    m_identifier = identifier;
}

void ExtForeignToplevelHandle::ext_foreign_toplevel_handle_v1_done() {
    // See m_readySent's declaration: `done` can legitimately fire again
    // later (e.g. once Biome pushes this window's real title/app_id, right
    // after creating this handle with a blank one) - only the first one
    // means the identifier (sent once, at creation) is ready.
    if (m_readySent) {
        return;
    }
    m_readySent = true;
    emit ready(this);
}

void ExtForeignToplevelHandle::ext_foreign_toplevel_handle_v1_closed() {
    emit closed(this);
}
