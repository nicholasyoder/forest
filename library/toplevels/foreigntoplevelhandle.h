// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FOREIGNTOPLEVELHANDLE_H
#define FOREIGNTOPLEVELHANDLE_H

#include <QObject>
#include <QString>

#include "qwayland-wlr-foreign-toplevel-management-unstable-v1.h"

// Caches title/app_id/state and emits changed() on each `done`.
class ForeignToplevelHandle : public QObject, public QtWayland::zwlr_foreign_toplevel_handle_v1{
    Q_OBJECT

public:
    ForeignToplevelHandle(struct ::zwlr_foreign_toplevel_handle_v1 *object, QObject *parent);
    ~ForeignToplevelHandle();

    QString title() const{return m_title;}
    QString appId() const{return m_appId;}
    bool isMaximized() const{return m_maximized;}
    bool isMinimized() const{return m_minimized;}
    bool isActivated() const{return m_activated;}

    // ext-foreign-toplevel-list identifier, paired in by ToplevelTracker; empty
    // until paired or when org.biome isn't available.
    QString identifier() const{return m_identifier;}
    void setIdentifier(const QString &identifier){m_identifier = identifier;}

    void activate();
    void setMaximized();
    void unsetMaximized();
    void setMinimized();
    void unsetMinimized();
    void requestClose();

signals:
    void changed(ForeignToplevelHandle *handle);
    void closed(ForeignToplevelHandle *handle);

protected:
    void zwlr_foreign_toplevel_handle_v1_title(const QString &title) override;
    void zwlr_foreign_toplevel_handle_v1_app_id(const QString &app_id) override;
    void zwlr_foreign_toplevel_handle_v1_state(wl_array *state) override;
    void zwlr_foreign_toplevel_handle_v1_done() override;
    void zwlr_foreign_toplevel_handle_v1_closed() override;

private:
    QString m_title;
    QString m_appId;
    QString m_identifier;
    bool m_maximized = false;
    bool m_minimized = false;
    bool m_activated = false;
};

#endif // FOREIGNTOPLEVELHANDLE_H
