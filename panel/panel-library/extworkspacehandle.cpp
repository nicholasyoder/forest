// SPDX-License-Identifier: LGPL-3.0-or-later

#include "extworkspacehandle.h"

#include "extworkspacemanager.h"

ExtWorkspaceHandle::ExtWorkspaceHandle(struct ::ext_workspace_handle_v1 *object, ExtWorkspaceManager *manager)
    : QtWayland::ext_workspace_handle_v1(object), m_manager(manager) {
}

ExtWorkspaceHandle::~ExtWorkspaceHandle() {
    destroy();
}

void ExtWorkspaceHandle::activate() {
    QtWayland::ext_workspace_handle_v1::activate();
    m_manager->commit();
}

void ExtWorkspaceHandle::ext_workspace_handle_v1_name(const QString &name) {
    m_name = name;
}

void ExtWorkspaceHandle::ext_workspace_handle_v1_state(uint32_t state) {
    m_active = state & state_active;
}

void ExtWorkspaceHandle::ext_workspace_handle_v1_removed() {
    emit removed(this);
}
