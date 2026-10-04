// SPDX-License-Identifier: LGPL-3.0-or-later

#include "extworkspacemanager.h"

#include "extworkspacehandle.h"

// Only kept so the group proxy exists to receive events and is destroyed
// when the compositor removes it; its output/workspace membership is unused.
class ExtWorkspaceGroup : public QtWayland::ext_workspace_group_handle_v1 {
public:
    ExtWorkspaceGroup(struct ::ext_workspace_group_handle_v1 *object, ExtWorkspaceManager *manager)
        : QtWayland::ext_workspace_group_handle_v1(object), m_manager(manager) {
    }
    ~ExtWorkspaceGroup() {
        destroy();
    }

protected:
    void ext_workspace_group_handle_v1_removed() override {
        m_manager->removeGroup(this);
    }

private:
    ExtWorkspaceManager *m_manager;
};

ExtWorkspaceManager::ExtWorkspaceManager(QObject *parent)
    : QWaylandClientExtensionTemplate<ExtWorkspaceManager>(1) {
    setParent(parent);
}

ExtWorkspaceManager::~ExtWorkspaceManager() {
    qDeleteAll(m_groups);
}

void ExtWorkspaceManager::release() {
    setParent(nullptr);
    if (isActive())
        stop();
    else
        deleteLater();
}

void ExtWorkspaceManager::ext_workspace_manager_v1_workspace_group(
        struct ::ext_workspace_group_handle_v1 *workspace_group) {
    m_groups << new ExtWorkspaceGroup(workspace_group, this);
}

void ExtWorkspaceManager::ext_workspace_manager_v1_workspace(struct ::ext_workspace_handle_v1 *workspace) {
    auto *handle = new ExtWorkspaceHandle(workspace, this);
    connect(handle, &ExtWorkspaceHandle::removed, this, &ExtWorkspaceManager::onWorkspaceRemoved);
    m_workspaces << handle;
}

void ExtWorkspaceManager::ext_workspace_manager_v1_done() {
    emit workspacesChanged();
}

void ExtWorkspaceManager::ext_workspace_manager_v1_finished() {
    // `finished` is a destructor event; libwayland leaves freeing the proxy to us.
    wl_proxy_destroy(reinterpret_cast<wl_proxy *>(object()));
    deleteLater();
}

void ExtWorkspaceManager::onWorkspaceRemoved(ExtWorkspaceHandle *handle) {
    m_workspaces.removeAll(handle);
    handle->deleteLater();
}

void ExtWorkspaceManager::removeGroup(ExtWorkspaceGroup *group) {
    m_groups.removeAll(group);
    delete group;
}

int ExtWorkspaceManager::activeWorkspaceIndex() const {
    for (int index = 0; index < m_workspaces.length(); index++){
        if (m_workspaces[index]->isActive())
            return index;
    }
    return -1;
}
