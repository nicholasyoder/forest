// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EXTWORKSPACEHANDLE_H
#define EXTWORKSPACEHANDLE_H

#include <QObject>
#include <QString>

#include "qwayland-ext-workspace-v1.h"

class ExtWorkspaceManager;

// No per-handle `done`; ExtWorkspaceManager::workspacesChanged() marks a
// consistent snapshot.
class ExtWorkspaceHandle : public QObject, public QtWayland::ext_workspace_handle_v1 {
    Q_OBJECT

public:
    ExtWorkspaceHandle(struct ::ext_workspace_handle_v1 *object, ExtWorkspaceManager *manager);
    ~ExtWorkspaceHandle();

    QString name() const { return m_name; }
    bool isActive() const { return m_active; }

    // Sends `activate` plus the manager's `commit`, without which it has no effect.
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
