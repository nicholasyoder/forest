// SPDX-License-Identifier: LGPL-3.0-or-later

#include "extforeigntoplevelhandle.h"

ExtForeignToplevelHandle::ExtForeignToplevelHandle(struct ::ext_foreign_toplevel_handle_v1 *object, QObject *parent)
    : QObject(parent), QtWayland::ext_foreign_toplevel_handle_v1(object) {
}

ExtForeignToplevelHandle::~ExtForeignToplevelHandle() {
    destroy();
}

void ExtForeignToplevelHandle::ext_foreign_toplevel_handle_v1_identifier(const QString &identifier) {
    m_identifier = identifier;
}

void ExtForeignToplevelHandle::ext_foreign_toplevel_handle_v1_done() {
    if (m_readySent)
        return;
    m_readySent = true;
    emit ready(this);
}

void ExtForeignToplevelHandle::ext_foreign_toplevel_handle_v1_closed() {
    emit closed(this);
}
