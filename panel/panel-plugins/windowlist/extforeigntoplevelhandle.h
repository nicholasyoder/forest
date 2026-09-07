// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EXTFOREIGNTOPLEVELHANDLE_H
#define EXTFOREIGNTOPLEVELHANDLE_H

#include <QObject>
#include <QString>

#include "qwayland-ext-foreign-toplevel-list-v1.h"

// Wraps one ext_foreign_toplevel_handle_v1 - created purely to obtain the
// stable `identifier` string this window has no other way to expose (see
// extforeigntoplevellist.h's pairing note). title/app_id are deliberately
// not tracked here - ForeignToplevelHandle/windowbutton already get those
// from wlr-foreign-toplevel-management.
class ExtForeignToplevelHandle : public QObject, public QtWayland::ext_foreign_toplevel_handle_v1 {
    Q_OBJECT

public:
    explicit ExtForeignToplevelHandle(struct ::ext_foreign_toplevel_handle_v1 *object);
    ~ExtForeignToplevelHandle();

    QString identifier() const { return m_identifier; }

signals:
    // Emitted once `done` confirms the identifier (sent right after
    // creation) has arrived.
    void ready(ExtForeignToplevelHandle *handle);
    void closed(ExtForeignToplevelHandle *handle);

protected:
    void ext_foreign_toplevel_handle_v1_identifier(const QString &identifier) override;
    void ext_foreign_toplevel_handle_v1_done() override;
    void ext_foreign_toplevel_handle_v1_closed() override;

private:
    QString m_identifier;
    // `done` is a batch-commit signal, not a one-time "ready" event - the
    // protocol re-sends it after every later change to title/app_id (see
    // ext_foreign_toplevel_handle_v1's `done` doc comment), which happens
    // for virtually every real window shortly after creation (Biome's
    // foreign_toplevel_create() first creates this handle with a blank
    // title/app_id, then pushes the real one moments later via
    // wlr_ext_foreign_toplevel_handle_v1_update_state(), each triggering its
    // own `done`). windowlist.cpp only wants the `identifier` (sent exactly
    // once, at creation, and never re-sent - see that event's own doc
    // comment) and deletes this handle right after its first `ready`, so
    // without this guard a second `done` re-emits `ready` for an object
    // that's about to be (or already was) freed - a real use-after-free
    // crash confirmed via a segfault backtrace landing in
    // windowlist::tryPairPendingHandles's extHandle->deleteLater() call.
    bool m_readySent = false;
};

#endif // EXTFOREIGNTOPLEVELHANDLE_H
