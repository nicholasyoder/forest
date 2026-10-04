// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Async client for org.freedesktop.portal.GlobalShortcuts. Each request
// replies later via org.freedesktop.portal.Request::Response.

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

    void createSession(std::function<void(bool ok)> onReady);
    void bindShortcuts(const QList<globalhotkey *> &hotkeys, std::function<void(bool ok)> onDone);
    // onClosed runs once the Close call returns, whatever its result.
    void closeSession(std::function<void()> onClosed);

signals:
    void shortcutActivated(QString id);
    // The portal closed our session or went away; in-flight requests fail first.
    void sessionLost();
    void portalAvailable();

private slots:
    void handleActivated(const QDBusObjectPath &session_handle, const QString &shortcut_id,
        qulonglong timestamp, const QVariantMap &options);
    void handleClosed(const QVariantMap &details);

private:
    // Subscribes to the Request's Response *before* sending `call`: the
    // Response can otherwise arrive before the match rule exists.
    void sendRequest(const QDBusMessage &call, const QString &handleToken, std::function<void(bool ok)> then);

    // Also moves the Session::Closed subscription to the new handle.
    void setSessionHandle(const QDBusObjectPath &handle);

    QDBusObjectPath m_sessionHandle;
};

#endif // GLOBALSHORTCUTSPORTAL_H
