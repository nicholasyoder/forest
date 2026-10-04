// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TOPLEVELTRACKER_H
#define TOPLEVELTRACKER_H

#include <QObject>
#include <QList>
#include <QPointer>

#include "foreigntoplevelmanager.h"
#include "foreigntoplevelhandle.h"
#include "extforeigntoplevellist.h"
#include "extforeigntoplevelhandle.h"
#include "extworkspacemanager.h"
#include "biomeworkspaces.h"

// Open windows plus which workspace each is on. Handles are deleted after
// their `closed` signal; connect to it to drop references.
class ToplevelTracker : public QObject {
    Q_OBJECT

public:
    explicit ToplevelTracker(QObject *parent = nullptr);
    ~ToplevelTracker();

    const QList<ForeignToplevelHandle *> &toplevels() const { return m_toplevels; }
    ExtWorkspaceManager *workspaceManager() const { return m_workspaceManager; }
    BiomeWorkspaces *biomeWorkspaces() const { return m_biomeWorkspaces; }

    // True for windows Biome hasn't classified, or with no active workspace known.
    bool isOnActiveWorkspace(ForeignToplevelHandle *handle) const;

signals:
    void toplevelAdded(ForeignToplevelHandle *handle);
    // Workspace state, the window -> workspace map, or a handle's identifier changed.
    void workspaceMembershipChanged();

private slots:
    void onToplevelCreated(ForeignToplevelHandle *handle);
    void onToplevelClosed(ForeignToplevelHandle *handle);
    void onExtToplevelCreated(ExtForeignToplevelHandle *handle);
    void onExtToplevelReady(ExtForeignToplevelHandle *handle);
    void onExtToplevelClosed(ExtForeignToplevelHandle *handle);

private:
    void tryPairPendingHandles();

    ForeignToplevelManager *m_toplevelManager = nullptr;
    ExtForeignToplevelList *m_extToplevelList = nullptr;
    ExtWorkspaceManager *m_workspaceManager = nullptr;
    BiomeWorkspaces *m_biomeWorkspaces = nullptr;

    QList<ForeignToplevelHandle *> m_toplevels;
    // Paired front-to-front; see extforeigntoplevellist.h.
    QList<QPointer<ForeignToplevelHandle>> m_pendingZwlrHandles;
    QList<QPointer<ExtForeignToplevelHandle>> m_pendingExtHandles;
};

#endif // TOPLEVELTRACKER_H
