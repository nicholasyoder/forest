// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EXTFOREIGNTOPLEVELHANDLE_H
#define EXTFOREIGNTOPLEVELHANDLE_H

#include <QObject>
#include <QString>

#include "qwayland-ext-foreign-toplevel-list-v1.h"

// Tracks `identifier` and is the capture-source key; title/app_id come from ForeignToplevelHandle.
class ExtForeignToplevelHandle : public QObject, public QtWayland::ext_foreign_toplevel_handle_v1 {
    Q_OBJECT

public:
    ExtForeignToplevelHandle(struct ::ext_foreign_toplevel_handle_v1 *object, QObject *parent);
    ~ExtForeignToplevelHandle();

    QString identifier() const { return m_identifier; }

signals:
    // Emitted on the first `done`, once `identifier` has arrived.
    void ready(ExtForeignToplevelHandle *handle);
    void closed(ExtForeignToplevelHandle *handle);

protected:
    void ext_foreign_toplevel_handle_v1_identifier(const QString &identifier) override;
    void ext_foreign_toplevel_handle_v1_done() override;
    void ext_foreign_toplevel_handle_v1_closed() override;

private:
    QString m_identifier;
    // `done` repeats on every title/app_id change; ToplevelTracker pairs the
    // handle on ready(), so it must only fire once.
    bool m_readySent = false;
};

#endif // EXTFOREIGNTOPLEVELHANDLE_H
