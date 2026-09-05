// SPDX-License-Identifier: LGPL-3.0-or-later

#include "extworkspacemanager.h"

#include "extworkspacehandle.h"

// Matches the forward declaration of ExtWorkspaceManager::m_group in
// extworkspacemanager.h - defined at file scope (not inside an anonymous
// namespace) so it's the same type as that field's pointee.
//
// Minimal wrapper for ext_workspace_group_handle_v1 - Biome advertises
// exactly one group (see extworkspacemanager.h) and never sends
// output_enter/leave (its workspace model is global, not per-output) or
// workspace_enter/leave beyond the initial burst, so there is nothing
// meaningful to surface from it; it only needs to exist so the protocol
// object has somewhere to receive events without erroring.
class ExtWorkspaceGroup : public QtWayland::ext_workspace_group_handle_v1 {
public:
    explicit ExtWorkspaceGroup(struct ::ext_workspace_group_handle_v1 *object)
        : QtWayland::ext_workspace_group_handle_v1(object) {
    }
    ~ExtWorkspaceGroup() {
        destroy();
    }
};

ExtWorkspaceManager::ExtWorkspaceManager() : QWaylandClientExtensionTemplate<ExtWorkspaceManager>(1) {
}

void ExtWorkspaceManager::ext_workspace_manager_v1_workspace_group(
        struct ::ext_workspace_group_handle_v1 *workspace_group) {
    delete m_group;
    m_group = new ExtWorkspaceGroup(workspace_group);
}

void ExtWorkspaceManager::ext_workspace_manager_v1_workspace(struct ::ext_workspace_handle_v1 *workspace) {
    auto *handle = new ExtWorkspaceHandle(workspace, this);
    connect(handle, &ExtWorkspaceHandle::removed, this, &ExtWorkspaceManager::onWorkspaceRemoved);
    m_workspaces << handle;
}

void ExtWorkspaceManager::ext_workspace_manager_v1_done() {
    emit workspacesChanged();
}

void ExtWorkspaceManager::onWorkspaceRemoved(ExtWorkspaceHandle *handle) {
    m_workspaces.removeAll(handle);
    handle->deleteLater();
}

int ExtWorkspaceManager::activeWorkspaceIndex() const {
    for (int index = 0; index < m_workspaces.length(); index++){
        if (m_workspaces[index]->isActive())
            return index;
    }
    return -1;
}
