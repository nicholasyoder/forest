// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EXTWORKSPACEMANAGER_H
#define EXTWORKSPACEMANAGER_H

#include <QWaylandClientExtension>
#include <QList>

#include "qwayland-ext-workspace-v1.h"

class ExtWorkspaceHandle;
class ExtWorkspaceGroup;

// State changes are atomic per manager-level `done`, surfaced as
// workspacesChanged(). Binding replays the full workspace list. Workspaces
// from all groups are flattened into one list (Biome has a single group).
class ExtWorkspaceManager : public QWaylandClientExtensionTemplate<ExtWorkspaceManager>,
                            public QtWayland::ext_workspace_manager_v1 {
    Q_OBJECT

public:
    explicit ExtWorkspaceManager(QObject *parent = nullptr);
    ~ExtWorkspaceManager();

    // Detaches from the parent, sends `stop`, and deletes itself (and all
    // handles) on `finished`.
    void release();

    const QList<ExtWorkspaceHandle *> &workspaces() const { return m_workspaces; }

    // -1 before the first workspacesChanged().
    int activeWorkspaceIndex() const;

signals:
    void workspacesChanged();

protected:
    void ext_workspace_manager_v1_workspace_group(struct ::ext_workspace_group_handle_v1 *workspace_group) override;
    void ext_workspace_manager_v1_workspace(struct ::ext_workspace_handle_v1 *workspace) override;
    void ext_workspace_manager_v1_done() override;
    void ext_workspace_manager_v1_finished() override;

private slots:
    void onWorkspaceRemoved(ExtWorkspaceHandle *handle);

private:
    friend class ExtWorkspaceGroup;
    void removeGroup(ExtWorkspaceGroup *group);

    QList<ExtWorkspaceGroup *> m_groups;
    QList<ExtWorkspaceHandle *> m_workspaces;
};

#endif // EXTWORKSPACEMANAGER_H
