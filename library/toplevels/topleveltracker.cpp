// SPDX-License-Identifier: LGPL-3.0-or-later

#include "topleveltracker.h"

#include "toplevelcapture.h"

ToplevelTracker::ToplevelTracker(QObject *parent, bool trackWorkspaces) : QObject(parent) {
    if (trackWorkspaces) {
        m_workspaces = new WorkspaceTracker(this);
        connect(m_workspaces, &WorkspaceTracker::changed, this, &ToplevelTracker::workspaceMembershipChanged);
    }

    ToplevelCapture::preload();

    m_toplevelManager = new ForeignToplevelManager(this);
    connect(m_toplevelManager, &ForeignToplevelManager::toplevelCreated, this, &ToplevelTracker::onToplevelCreated);

    // Identifiers (org.biome.Workspaces) and capture sources both need the ext
    // handle. Bound together with m_toplevelManager so both replays line up
    // for pairing.
    m_extToplevelList = new ExtForeignToplevelList(this);
    connect(m_extToplevelList, &ExtForeignToplevelList::toplevelCreated, this, &ToplevelTracker::onExtToplevelCreated);
}

ToplevelTracker::~ToplevelTracker() {
    // Handles are the managers' children, freed once each manager finishes.
    m_toplevelManager->release();
    if (m_extToplevelList)
        m_extToplevelList->release();
}

bool ToplevelTracker::isOnActiveWorkspace(ForeignToplevelHandle *handle) const {
    int active = m_workspaces ? m_workspaces->activeIndex() : -1;
    if (active < 0)
        return true;
    int index = m_workspaces->indexOfWindow(handle->identifier());
    return index < 0 || index == active;
}

void ToplevelTracker::onToplevelCreated(ForeignToplevelHandle *handle) {
    connect(handle, &ForeignToplevelHandle::closed, this, &ToplevelTracker::onToplevelClosed);
    m_toplevels << handle;

    // Queued now, not on first `done`: pairing depends on creation order.
    if (m_extToplevelList) {
        m_pendingZwlrHandles << handle;
        tryPairPendingHandles();
    }
    emit toplevelAdded(handle);
}

void ToplevelTracker::onToplevelClosed(ForeignToplevelHandle *handle) {
    m_toplevels.removeAll(handle);
    m_pendingZwlrHandles.removeAll(handle);
    handle->deleteLater();
}

void ToplevelTracker::onExtToplevelCreated(ExtForeignToplevelHandle *handle) {
    connect(handle, &ExtForeignToplevelHandle::ready, this, &ToplevelTracker::onExtToplevelReady);
    connect(handle, &ExtForeignToplevelHandle::closed, this, &ToplevelTracker::onExtToplevelClosed);
}

void ToplevelTracker::onExtToplevelReady(ExtForeignToplevelHandle *handle) {
    m_pendingExtHandles << handle;
    tryPairPendingHandles();
}

void ToplevelTracker::onExtToplevelClosed(ExtForeignToplevelHandle *handle) {
    m_pendingExtHandles.removeAll(handle);
    handle->deleteLater();
}

void ToplevelTracker::tryPairPendingHandles() {
    bool paired = false;
    while (!m_pendingZwlrHandles.isEmpty() && !m_pendingExtHandles.isEmpty()) {
        QPointer<ForeignToplevelHandle> zwlrHandle = m_pendingZwlrHandles.takeFirst();
        QPointer<ExtForeignToplevelHandle> extHandle = m_pendingExtHandles.takeFirst();
        // QPointer: fail safe if a stale entry is ever re-queued.
        // The ext handle stays alive (until its `closed`) for capture.
        if (zwlrHandle && extHandle) {
            zwlrHandle->setIdentifier(extHandle->identifier());
            zwlrHandle->setExtHandle(extHandle);
        } else if (extHandle) {
            extHandle->deleteLater();
        }
        paired = true;
    }
    if (paired)
        emit workspaceMembershipChanged();
}
