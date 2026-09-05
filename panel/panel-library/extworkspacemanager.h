// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EXTWORKSPACEMANAGER_H
#define EXTWORKSPACEMANAGER_H

#include <QWaylandClientExtension>
#include <QList>

#include "qwayland-ext-workspace-v1.h"

class ExtWorkspaceHandle;
class ExtWorkspaceGroup;

// Binds ext_workspace_manager_v1. Biome advertises a single workspace group
// spanning all outputs (see biome/docs/phase4-plan.md Workstream D) with a
// fixed set of workspaces created once at startup, so this class doesn't
// track multiple groups.
//
// Unlike zwlr_foreign_toplevel_handle_v1 (one `done` per toplevel),
// ext-workspace-v1 batches ALL state changes - the initial enumeration and
// every later activate - behind one manager-level `done` event (see the
// protocol's own commit/done description). workspacesChanged() is the
// single signal callers should use: by the time it fires, every handle in
// workspaces() reflects a fully consistent, atomic snapshot. No manual
// seeding is needed either: binding the manager makes the compositor replay
// the full workspace list immediately, same shape as
// ForeignToplevelManager's toplevel replay.
class ExtWorkspaceManager : public QWaylandClientExtensionTemplate<ExtWorkspaceManager>,
                            public QtWayland::ext_workspace_manager_v1 {
    Q_OBJECT

public:
    ExtWorkspaceManager();

    const QList<ExtWorkspaceHandle *> &workspaces() const { return m_workspaces; }

    // Index into workspaces() of the currently active one, or -1 if none is
    // marked active yet (before the initial workspacesChanged()).
    int activeWorkspaceIndex() const;

signals:
    // Fired once after the initial workspace burst, and again after every
    // subsequent `done` (e.g. following an activate()/commit() round trip)
    // - see this class's header comment.
    void workspacesChanged();

protected:
    void ext_workspace_manager_v1_workspace_group(struct ::ext_workspace_group_handle_v1 *workspace_group) override;
    void ext_workspace_manager_v1_workspace(struct ::ext_workspace_handle_v1 *workspace) override;
    void ext_workspace_manager_v1_done() override;

private slots:
    void onWorkspaceRemoved(ExtWorkspaceHandle *handle);

private:
    ExtWorkspaceGroup *m_group = nullptr;
    QList<ExtWorkspaceHandle *> m_workspaces;
};

#endif // EXTWORKSPACEMANAGER_H
