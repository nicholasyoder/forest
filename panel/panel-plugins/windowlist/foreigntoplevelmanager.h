// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FOREIGNTOPLEVELMANAGER_H
#define FOREIGNTOPLEVELMANAGER_H

#include <QWaylandClientExtension>

#include "qwayland-wlr-foreign-toplevel-management-unstable-v1.h"

class ForeignToplevelHandle;

// Binds zwlr_foreign_toplevel_manager_v1. The compositor sends one
// `toplevel` event per already-open window as soon as the binding becomes
// active, then one more each time a new window is mapped - so no manual
// "seed existing windows" step is needed, unlike KX11Extras::windows().
class ForeignToplevelManager : public QWaylandClientExtensionTemplate<ForeignToplevelManager>, public QtWayland::zwlr_foreign_toplevel_manager_v1{
    Q_OBJECT

public:
    ForeignToplevelManager();

signals:
    void toplevelCreated(ForeignToplevelHandle *handle);

protected:
    void zwlr_foreign_toplevel_manager_v1_toplevel(struct ::zwlr_foreign_toplevel_handle_v1 *toplevel) override;
};

#endif // FOREIGNTOPLEVELMANAGER_H
