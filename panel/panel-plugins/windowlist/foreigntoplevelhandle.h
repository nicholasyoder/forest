// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FOREIGNTOPLEVELHANDLE_H
#define FOREIGNTOPLEVELHANDLE_H

#include <QObject>
#include <QString>

#include "qwayland-wlr-foreign-toplevel-management-unstable-v1.h"

// Wraps one zwlr_foreign_toplevel_handle_v1 - one per open window, created
// by ForeignToplevelManager::zwlr_foreign_toplevel_manager_v1_toplevel() and
// destroyed when the compositor sends `closed`. Title/app_id/state are
// cached here as they arrive and only surfaced via changed() once `done`
// confirms a batch of updates is complete, matching the shape
// windowlist::onWindowChanged already expects from KX11Extras::windowChanged.
class ForeignToplevelHandle : public QObject, public QtWayland::zwlr_foreign_toplevel_handle_v1{
    Q_OBJECT

public:
    explicit ForeignToplevelHandle(struct ::zwlr_foreign_toplevel_handle_v1 *object);
    ~ForeignToplevelHandle();

    QString title() const{return m_title;}
    QString appId() const{return m_appId;}
    bool isMaximized() const{return m_maximized;}
    bool isMinimized() const{return m_minimized;}
    bool isActivated() const{return m_activated;}

    void activate();
    void setMaximized();
    void unsetMaximized();
    void setMinimized();
    void requestClose();

signals:
    // Emitted after a batch of title/app_id/state updates is complete.
    // Carries `this` so one shared slot (windowlist::onWindowChanged) can
    // tell which handle fired without needing per-handle lambdas.
    void changed(ForeignToplevelHandle *handle);
    // The compositor destroyed this toplevel.
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
    bool m_maximized = false;
    bool m_minimized = false;
    bool m_activated = false;
};

#endif // FOREIGNTOPLEVELHANDLE_H
