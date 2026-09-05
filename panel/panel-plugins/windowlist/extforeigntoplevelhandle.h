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
};

#endif // EXTFOREIGNTOPLEVELHANDLE_H
