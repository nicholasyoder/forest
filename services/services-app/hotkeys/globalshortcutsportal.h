// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Client for the real org.freedesktop.portal.GlobalShortcuts frontend
// interface on org.freedesktop.portal.Desktop
// (/org/freedesktop/portal/desktop), brokered by xdg-desktop-portal to
// Biome's org.freedesktop.impl.portal.GlobalShortcuts backend
// (biome/ipc/global_shortcuts_portal.cpp) - see docs/phase4-session-log.md's
// 2026-08-26 Workstream C step 2 entry in the biome repo for the validated
// contract this is written against: CreateSession/BindShortcuts/
// ListShortcuts are async, each returning only a Request object path that
// replies later via that object's org.freedesktop.portal.Request::Response
// signal; Activated/Deactivated are `osta{sv}` signals on the
// GlobalShortcuts interface itself.

#ifndef GLOBALSHORTCUTSPORTAL_H
#define GLOBALSHORTCUTSPORTAL_H

#include "hotkey.h"

#include <QtDBus>

#include <functional>

class GlobalShortcutsPortal : public QObject
{
    Q_OBJECT

public:
    explicit GlobalShortcutsPortal(QObject *parent = nullptr);

    // Creates a fresh portal session. Calls onReady(true) once
    // session_handle is populated and ready for bindShortcuts(), or
    // onReady(false) if the CreateSession request itself failed or was
    // denied.
    void createSession(std::function<void(bool ok)> onReady);

    // Binds every hotkey's triggerString() under the current session.
    // onDone(true) once the daemon replies with success (whether or not
    // every individual trigger was accepted - Biome auto-accepts with no
    // confirmation UI, see the backend's own header comment), onDone(false)
    // on a request-level failure.
    void bindShortcuts(const QList<globalhotkey *> &hotkeys, std::function<void(bool ok)> onDone);

    // Calls Close() on the current session's org.freedesktop.portal.Session
    // object, then calls onClosed() once that call returns (regardless of
    // its result - closing an already-gone session isn't an error worth
    // failing a reload over).
    void closeSession(std::function<void()> onClosed);

signals:
    // Re-emitted from the GlobalShortcuts interface's own Activated signal
    // once connected at construction (not per-request) - foresthotkeys
    // connects to this to dispatch by id.
    void shortcutActivated(QString id);

private slots:
    // Connected once, at construction, directly to the GlobalShortcuts
    // interface's real Activated signal (`osta{sv}`) - not per-request.
    // Deactivated is intentionally not connected: every bound shortcut is
    // instant press+release per Biome's prototype, so it's currently
    // unused.
    void handleActivated(const QDBusObjectPath &session_handle, const QString &shortcut_id,
        qulonglong timestamp, const QVariantMap &options);

private:
    QString newHandleToken();

    // Runs `then` once the Request object at `path` emits its Response
    // signal, then disconnects and deletes the temporary connection.
    void awaitResponse(const QDBusObjectPath &path, std::function<void(uint code, const QVariantMap &results)> then);

    QDBusObjectPath m_sessionHandle;
};

#endif // GLOBALSHORTCUTSPORTAL_H
