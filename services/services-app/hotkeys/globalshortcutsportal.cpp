// SPDX-License-Identifier: LGPL-3.0-or-later

#include "globalshortcutsportal.h"

namespace {

constexpr char kBusService[] = "org.freedesktop.portal.Desktop";
constexpr char kObjectPath[] = "/org/freedesktop/portal/desktop";
constexpr char kGlobalShortcutsIface[] = "org.freedesktop.portal.GlobalShortcuts";
constexpr char kRequestIface[] = "org.freedesktop.portal.Request";
constexpr char kSessionIface[] = "org.freedesktop.portal.Session";

// QDBusConnection::connect() needs a real slot; this forwards one Request's
// outcome to a callback, exactly once. Its bus match goes away with it.
class PortalRequest : public QObject {
    Q_OBJECT

public:
    explicit PortalRequest(std::function<void(bool)> callback, QObject *parent = nullptr)
        : QObject(parent), m_callback(std::move(callback)) {
    }

    void finish(bool ok) {
        if (m_done) return;
        m_done = true;
        m_callback(ok);
        deleteLater();
    }

public slots:
    void onResponse(uint code, const QVariantMap &) {
        if (code != 0) qWarning() << "GlobalShortcutsPortal: request failed, response code" << code;
        finish(code == 0);
    }

private:
    std::function<void(bool)> m_callback;
    bool m_done = false;
};

// One entry of the portal's `a(sa{sv})` shortcuts array.
struct PortalShortcutSpec {
    QString id;
    QVariantMap options;
};

} // namespace

Q_DECLARE_METATYPE(PortalShortcutSpec)

namespace {

QDBusArgument &operator<<(QDBusArgument &arg, const PortalShortcutSpec &spec) {
    arg.beginStructure();
    arg << spec.id << spec.options;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, PortalShortcutSpec &spec) {
    arg.beginStructure();
    arg >> spec.id >> spec.options;
    arg.endStructure();
    return arg;
}

QString newHandleToken() {
    static int counter = 0;
    return QStringLiteral("forest_hotkeys_%1").arg(counter++);
}

// Unique bus name mangled per the portal object-path convention.
QString escapedSender() {
    QString sender = QDBusConnection::sessionBus().baseService();
    sender.remove(0, 1);
    sender.replace('.', '_');
    return sender;
}

QDBusMessage portalCall(const QString &method) {
    return QDBusMessage::createMethodCall(kBusService, kObjectPath, kGlobalShortcutsIface, method);
}

} // namespace

GlobalShortcutsPortal::GlobalShortcutsPortal(QObject *parent) : QObject(parent) {
    qDBusRegisterMetaType<PortalShortcutSpec>();
    qDBusRegisterMetaType<QList<PortalShortcutSpec>>();

    const bool ok = QDBusConnection::sessionBus().connect(
        QString(kBusService), QString(kObjectPath), QString(kGlobalShortcutsIface),
        QStringLiteral("Activated"), this,
        SLOT(handleActivated(QDBusObjectPath, QString, qulonglong, QVariantMap)));
    if (!ok) {
        qWarning() << "GlobalShortcutsPortal: failed to connect to Activated signal:"
                   << QDBusConnection::sessionBus().lastError().message();
    }

    auto *watcher = new QDBusServiceWatcher(QString(kBusService), QDBusConnection::sessionBus(),
        QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this,
            [this](const QString &, const QString &oldOwner, const QString &newOwner) {
        // Ignore "" -> owner: our own CreateSession may be what activated it.
        if (!oldOwner.isEmpty()) {
            qWarning() << "GlobalShortcutsPortal: portal went away";
            setSessionHandle(QDBusObjectPath());
            // Their Responses will never arrive.
            for (PortalRequest *request : findChildren<PortalRequest *>(Qt::FindDirectChildrenOnly))
                request->finish(false);
            emit sessionLost();
        }
        if (!newOwner.isEmpty()) emit portalAvailable();
    });
}

void GlobalShortcutsPortal::setSessionHandle(const QDBusObjectPath &handle) {
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!m_sessionHandle.path().isEmpty()) {
        bus.disconnect(QString(kBusService), m_sessionHandle.path(), QString(kSessionIface),
            QStringLiteral("Closed"), this, SLOT(handleClosed(QVariantMap)));
    }
    m_sessionHandle = handle;
    if (!handle.path().isEmpty() && !bus.connect(QString(kBusService), handle.path(), QString(kSessionIface),
            QStringLiteral("Closed"), this, SLOT(handleClosed(QVariantMap)))) {
        qWarning() << "GlobalShortcutsPortal: failed to connect to Session::Closed at" << handle.path();
    }
}

void GlobalShortcutsPortal::sendRequest(const QDBusMessage &call, const QString &handleToken,
        std::function<void(bool ok)> then) {
    const QString path = QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2")
        .arg(escapedSender(), handleToken);

    auto *request = new PortalRequest(std::move(then), this);
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.connect(QString(kBusService), path, QString(kRequestIface), QStringLiteral("Response"),
            request, SLOT(onResponse(uint, QVariantMap)))) {
        qWarning() << "GlobalShortcutsPortal: failed to connect to Request::Response at" << path;
        request->finish(false);
        return;
    }

    auto *watcher = new QDBusPendingCallWatcher(bus.asyncCall(call), request);
    connect(watcher, &QDBusPendingCallWatcher::finished, request, [request, path, call](QDBusPendingCallWatcher *w) {
        const QDBusPendingReply<QDBusObjectPath> reply = *w;
        w->deleteLater();
        if (reply.isError()) {
            qWarning() << "GlobalShortcutsPortal:" << call.member() << "failed:" << reply.error().message();
            request->finish(false);
        } else if (reply.value().path() != path) {
            // Pre-1.0 portals chose their own path; we'd never see the Response.
            qWarning() << "GlobalShortcutsPortal: unexpected request path" << reply.value().path();
            request->finish(false);
        }
    });
}

void GlobalShortcutsPortal::createSession(std::function<void(bool ok)> onReady) {
    const QString handleToken = newHandleToken();
    const QString sessionToken = newHandleToken();
    QVariantMap options;
    options.insert(QStringLiteral("handle_token"), handleToken);
    options.insert(QStringLiteral("session_handle_token"), sessionToken);

    // Derived from our sender + token; it's not returned in the Response.
    setSessionHandle(QDBusObjectPath(
        QStringLiteral("/org/freedesktop/portal/desktop/session/%1/%2").arg(escapedSender(), sessionToken)));

    QDBusMessage call = portalCall(QStringLiteral("CreateSession"));
    call << options;
    sendRequest(call, handleToken, [this, onReady](bool ok) {
        if (!ok) setSessionHandle(QDBusObjectPath());
        onReady(ok);
    });
}

void GlobalShortcutsPortal::bindShortcuts(const QList<globalhotkey *> &hotkeys, std::function<void(bool ok)> onDone) {
    QList<PortalShortcutSpec> shortcuts;
    for (globalhotkey *item : hotkeys) {
        const QString trigger = item->triggerString();
        qInfo() << "GlobalShortcutsPortal: binding" << item->id() << "->" << trigger;
        if (trigger.isEmpty()) {
            continue; // triggerString() already logged why
        }
        PortalShortcutSpec spec;
        spec.id = item->id();
        spec.options.insert(QStringLiteral("description"), item->description());
        spec.options.insert(QStringLiteral("preferred_trigger"), trigger);
        shortcuts.append(spec);
    }

    const QString handleToken = newHandleToken();
    QVariantMap options;
    options.insert(QStringLiteral("handle_token"), handleToken);

    QDBusMessage call = portalCall(QStringLiteral("BindShortcuts"));
    call << QVariant::fromValue(m_sessionHandle) << QVariant::fromValue(shortcuts) << QString() << options;
    sendRequest(call, handleToken, std::move(onDone));
}

void GlobalShortcutsPortal::closeSession(std::function<void()> onClosed) {
    if (m_sessionHandle.path().isEmpty()) {
        onClosed();
        return;
    }

    const QDBusMessage call = QDBusMessage::createMethodCall(
        kBusService, m_sessionHandle.path(), kSessionIface, QStringLiteral("Close"));
    setSessionHandle(QDBusObjectPath());

    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(call), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [onClosed](QDBusPendingCallWatcher *w) {
        if (w->isError()) {
            qWarning() << "GlobalShortcutsPortal: Session.Close() failed:" << w->error().message();
        }
        w->deleteLater();
        onClosed();
    });
}

void GlobalShortcutsPortal::handleActivated(const QDBusObjectPath &session_handle, const QString &shortcut_id,
        qulonglong timestamp, const QVariantMap &options) {
    Q_UNUSED(timestamp);
    Q_UNUSED(options);
    if (session_handle != m_sessionHandle) {
        return; // stale signal from a session we've already torn down
    }
    emit shortcutActivated(shortcut_id);
}

void GlobalShortcutsPortal::handleClosed(const QVariantMap &details) {
    Q_UNUSED(details);
    qWarning() << "GlobalShortcutsPortal: portal closed our session";
    setSessionHandle(QDBusObjectPath());
    emit sessionLost();
}

#include "globalshortcutsportal.moc"
