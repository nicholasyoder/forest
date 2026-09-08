// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EXTWORKSPACEHANDLE_H
#define EXTWORKSPACEHANDLE_H

#include <QObject>
#include <QString>

#include "qwayland-ext-workspace-v1.h"

class ExtWorkspaceManager;

// Wraps one ext_workspace_handle_v1 - one per Biome workspace, created by
// ExtWorkspaceManager::ext_workspace_manager_v1_workspace(). Biome's
// workspace set is fixed at startup (no create_workspace/remove capability
// is ever advertised - see biome/docs/architecture-notes.md's "Workspace
// protocol (Workstream D)" section), so
// removed() is only here for spec completeness, not because it's expected
// to fire in practice.
//
// Unlike ForeignToplevelHandle/zwlr_foreign_toplevel_handle_v1, this
// protocol has no per-handle `done` event - atomicity is at the manager
// level instead (see ExtWorkspaceManager's header comment), so this class
// just caches each event as it arrives with no batching of its own.
class ExtWorkspaceHandle : public QObject, public QtWayland::ext_workspace_handle_v1 {
    Q_OBJECT

public:
    ExtWorkspaceHandle(struct ::ext_workspace_handle_v1 *object, ExtWorkspaceManager *manager);
    ~ExtWorkspaceHandle();

    QString name() const { return m_name; }
    bool isActive() const { return m_active; }

    // Issues the `activate` request followed by the manager's `commit` -
    // ext-workspace-v1 batches state changes at the manager level, so a
    // bare activate() request alone has no effect until commit() follows.
    void activate();

signals:
    void removed(ExtWorkspaceHandle *handle);

protected:
    void ext_workspace_handle_v1_name(const QString &name) override;
    void ext_workspace_handle_v1_state(uint32_t state) override;
    void ext_workspace_handle_v1_removed() override;

private:
    ExtWorkspaceManager *m_manager;
    QString m_name;
    bool m_active = false;
};

#endif // EXTWORKSPACEHANDLE_H
