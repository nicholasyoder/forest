// SPDX-License-Identifier: LGPL-3.0-or-later

#include "workspacetracker.h"

#include "biomeworkspaces.h"
#include "extworkspacehandle.h"
#include "extworkspacemanager.h"

WorkspaceTracker::WorkspaceTracker(QObject *parent) : QObject(parent) {
    m_biome = new BiomeWorkspaces(this);
    connect(m_biome, &BiomeWorkspaces::windowWorkspacesChanged, this, &WorkspaceTracker::changed);
    m_manager = new ExtWorkspaceManager(this);
    connect(m_manager, &ExtWorkspaceManager::workspacesChanged, this, &WorkspaceTracker::changed);
}

WorkspaceTracker::~WorkspaceTracker() {
    m_manager->release();
}

int WorkspaceTracker::count() const {
    return m_manager->workspaces().length();
}

QString WorkspaceTracker::name(int index) const {
    return index >= 0 && index < count() ? m_manager->workspaces()[index]->name() : QString();
}

int WorkspaceTracker::activeIndex() const {
    return m_manager->activeWorkspaceIndex();
}

void WorkspaceTracker::activate(int index) {
    if (index >= 0 && index < count())
        m_manager->workspaces()[index]->activate();
}

int WorkspaceTracker::numberOf(int index) const {
    bool ok = false;
    int number = m_manager->workspaces()[index]->id().toInt(&ok);
    return ok ? number : index;
}

int WorkspaceTracker::indexOfNumber(int number) const {
    for (int index = 0; index < count(); index++) {
        if (numberOf(index) == number)
            return index;
    }
    return -1;
}

int WorkspaceTracker::indexOfWindow(const QString &identifier) const {
    const QVariantMap &windows = m_biome->windowWorkspaces();
    auto it = windows.constFind(identifier);
    return it == windows.constEnd() ? -1 : indexOfNumber(it->toInt());
}

QList<int> WorkspaceTracker::windowCounts() const {
    QList<int> counts(count(), 0);
    for (const QVariant &number : m_biome->windowWorkspaces()) {
        int index = indexOfNumber(number.toInt());
        if (index >= 0)
            counts[index]++;
    }
    return counts;
}

bool WorkspaceTracker::canMoveWindows() const {
    return m_biome->isAvailable();
}

void WorkspaceTracker::moveWindow(const QString &identifier, int index) {
    if (!identifier.isEmpty() && index >= 0 && index < count())
        m_biome->moveToplevel(identifier, numberOf(index));
}
