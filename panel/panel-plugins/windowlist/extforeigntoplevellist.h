// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EXTFOREIGNTOPLEVELLIST_H
#define EXTFOREIGNTOPLEVELLIST_H

#include <QWaylandClientExtension>

#include "qwayland-ext-foreign-toplevel-list-v1.h"

class ExtForeignToplevelHandle;

// Binds ext_foreign_toplevel_list_v1, purely to hand windowlist a stable
// per-toplevel `identifier` string for org.biome.Workspaces'
// MoveToplevelToWorkspace DBus call - wlr-foreign-toplevel-management
// (ForeignToplevelManager/ForeignToplevelHandle) has no such identifier at
// all, and neither protocol cross-references the other.
//
// windowlist pairs the two handles for the same window by creation order:
// Biome creates both the wlr_ and ext_ handle for a given toplevel
// back-to-back in the same call (see biome/desktop/foreign_toplevel.cpp's
// foreign_toplevel_create()), so this manager's `toplevel` events arrive in
// the same relative order as ForeignToplevelManager's - see windowlist.cpp's
// pairing queue (tryPairPendingHandles()). Flagged in the Workstream D plan
// for manual multi-window stress testing, since this ordering guarantee
// isn't expressed anywhere in either protocol itself.
class ExtForeignToplevelList : public QWaylandClientExtensionTemplate<ExtForeignToplevelList>,
                                public QtWayland::ext_foreign_toplevel_list_v1 {
    Q_OBJECT

public:
    ExtForeignToplevelList();

signals:
    void toplevelCreated(ExtForeignToplevelHandle *handle);

protected:
    void ext_foreign_toplevel_list_v1_toplevel(struct ::ext_foreign_toplevel_handle_v1 *toplevel) override;
};

#endif // EXTFOREIGNTOPLEVELLIST_H
