// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SHOWDESKTOP_H
#define SHOWDESKTOP_H

#include <QObject>
#include <QList>

#include "topleveltracker.h"

// No protocol has a show-desktop request, so this minimizes the active
// workspace's windows itself and remembers them for the next toggle.
// Known limits: wlr-foreign-toplevel has no stacking order, so restored
// windows other than the previously active one may come back reordered;
// without org.biome.Workspaces every window counts as on the active workspace.
class ShowDesktop : public QObject {
    Q_OBJECT

public:
    explicit ShowDesktop(QObject *parent = nullptr);

    void setup();

public slots:
    // Called by D-Bus.
    void toggle();

private:
    struct Entry {
        ForeignToplevelHandle *handle;
        // Our set_minimized is async; until it lands, `!minimized` isn't a restore.
        bool seenMinimized;
    };

    void onToplevelAdded(ForeignToplevelHandle *handle);
    void onHandleChanged(ForeignToplevelHandle *handle);
    void onHandleClosed(ForeignToplevelHandle *handle);
    void onWorkspaceMembershipChanged();
    void clearRecord();

    ToplevelTracker *tracker = nullptr;
    QList<Entry> record;
    ForeignToplevelHandle *previouslyActive = nullptr;
    int recordWorkspace = -1;
};

#endif // SHOWDESKTOP_H
