// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FOREIGNTOPLEVELMANAGER_H
#define FOREIGNTOPLEVELMANAGER_H

#include <QWaylandClientExtension>

#include "qwayland-wlr-foreign-toplevel-management-unstable-v1.h"

class ForeignToplevelHandle;

// Binding replays a `toplevel` event for every already-open window. Handles
// are children of this object.
class ForeignToplevelManager : public QWaylandClientExtensionTemplate<ForeignToplevelManager>, public QtWayland::zwlr_foreign_toplevel_manager_v1{
    Q_OBJECT

public:
    explicit ForeignToplevelManager(QObject *parent = nullptr);

    // Detaches from the parent, sends `stop`, and deletes itself (and all
    // handles) on `finished`.
    void release();

signals:
    void toplevelCreated(ForeignToplevelHandle *handle);

protected:
    void zwlr_foreign_toplevel_manager_v1_toplevel(struct ::zwlr_foreign_toplevel_handle_v1 *toplevel) override;
    void zwlr_foreign_toplevel_manager_v1_finished() override;
};

#endif // FOREIGNTOPLEVELMANAGER_H
