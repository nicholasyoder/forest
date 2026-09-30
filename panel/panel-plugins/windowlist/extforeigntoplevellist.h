// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EXTFOREIGNTOPLEVELLIST_H
#define EXTFOREIGNTOPLEVELLIST_H

#include <QWaylandClientExtension>

#include "qwayland-ext-foreign-toplevel-list-v1.h"

class ExtForeignToplevelHandle;

// Bound only for the per-window `identifier` org.biome.Workspaces keys on.
// windowlist pairs these with wlr handles by creation order, which relies
// on Biome creating both handles back-to-back (neither protocol links them).
class ExtForeignToplevelList : public QWaylandClientExtensionTemplate<ExtForeignToplevelList>,
                                public QtWayland::ext_foreign_toplevel_list_v1 {
    Q_OBJECT

public:
    explicit ExtForeignToplevelList(QObject *parent = nullptr);

    // Detaches from the parent, sends `stop`, and deletes itself (and all
    // handles) on `finished`.
    void release();

signals:
    void toplevelCreated(ExtForeignToplevelHandle *handle);

protected:
    void ext_foreign_toplevel_list_v1_toplevel(struct ::ext_foreign_toplevel_handle_v1 *toplevel) override;
    void ext_foreign_toplevel_list_v1_finished() override;
};

#endif // EXTFOREIGNTOPLEVELLIST_H
