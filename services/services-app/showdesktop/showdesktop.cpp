// SPDX-License-Identifier: LGPL-3.0-or-later

#include "showdesktop.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

ShowDesktop::ShowDesktop(QObject *parent) : QObject(parent){
}

void ShowDesktop::setup(){
    if (!QDBusConnection::sessionBus().registerObject("/org/forest/showdesktop", this, QDBusConnection::ExportAllSlots))
        qCritical() << "Failed to register /org/forest/showdesktop on DBus:" << QDBusConnection::sessionBus().lastError().message();

    tracker = new ToplevelTracker(this);
    connect(tracker, &ToplevelTracker::toplevelAdded, this, &ShowDesktop::onToplevelAdded);
    connect(tracker, &ToplevelTracker::workspaceMembershipChanged, this, &ShowDesktop::onWorkspaceMembershipChanged);
}

void ShowDesktop::toggle(){
    if (!record.isEmpty()){
        // Taken first so the state changes our own requests cause aren't seen as outside restores.
        QList<Entry> restore = record;
        ForeignToplevelHandle *active = previouslyActive;
        clearRecord();
        for (const Entry &entry : restore)
            entry.handle->unsetMinimized();
        // Biome focuses each window it unminimizes, so re-focus the old one last.
        if (active)
            active->activate();
        return;
    }

    for (ForeignToplevelHandle *handle : tracker->toplevels()){
        if (handle->isMinimized() || !tracker->isOnActiveWorkspace(handle))
            continue;
        record << Entry{handle, false};
        if (handle->isActivated())
            previouslyActive = handle;
    }
    if (record.isEmpty())
        return;

    recordWorkspace = tracker->workspaceManager()->activeWorkspaceIndex();
    for (const Entry &entry : std::as_const(record))
        entry.handle->setMinimized();
}

void ShowDesktop::onToplevelAdded(ForeignToplevelHandle *handle){
    connect(handle, &ForeignToplevelHandle::changed, this, &ShowDesktop::onHandleChanged);
    connect(handle, &ForeignToplevelHandle::closed, this, &ShowDesktop::onHandleClosed);
    clearRecord();
}

void ShowDesktop::onHandleChanged(ForeignToplevelHandle *handle){
    for (Entry &entry : record){
        if (entry.handle != handle)
            continue;
        if (handle->isMinimized())
            entry.seenMinimized = true;
        else if (entry.seenMinimized)
            clearRecord(); // activated or unminimized by something else
        return;
    }
}

void ShowDesktop::onHandleClosed(ForeignToplevelHandle *handle){
    record.removeIf([handle](const Entry &entry){ return entry.handle == handle; });
    if (previouslyActive == handle)
        previouslyActive = nullptr;
}

void ShowDesktop::onWorkspaceMembershipChanged(){
    if (!record.isEmpty() && tracker->workspaceManager()->activeWorkspaceIndex() != recordWorkspace)
        clearRecord();
}

void ShowDesktop::clearRecord(){
    record.clear();
    previouslyActive = nullptr;
    recordWorkspace = -1;
}
