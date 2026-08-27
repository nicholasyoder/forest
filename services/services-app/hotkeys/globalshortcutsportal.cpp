// SPDX-License-Identifier: LGPL-3.0-or-later

#include "globalshortcutsportal.h"

namespace {

constexpr char kBusService[] = "org.freedesktop.portal.Desktop";
constexpr char kObjectPath[] = "/org/freedesktop/portal/desktop";
constexpr char kGlobalShortcutsIface[] = "org.freedesktop.portal.GlobalShortcuts";
constexpr char kRequestIface[] = "org.freedesktop.portal.Request";
constexpr char kSessionIface[] = "org.freedesktop.portal.Session";

// Forwards a Request object's one-shot Response signal to an arbitrary
// std::function - QDBusConnection::connect() needs a real Qt slot, so this
// is the shim that lets awaitResponse() take a lambda per call instead of
// one fixed slot per possible caller. Deletes itself once fired.
class PortalRequest : public QObject {
    Q_OBJECT

public:
    explicit PortalRequest(std::function<void(uint, const QVariantMap &)> callback, QObject *parent = nullptr)
        : QObject(parent), m_callback(std::move(callback)) {
    }

public slots:
    void onResponse(uint code, const QVariantMap &results) {
        m_callback(code, results);
        deleteLater();
    }

private:
    std::function<void(uint, const QVariantMap &)> m_callback;
};

// One entry of the portal's `a(sa{sv})` shortcuts array - mirrors
// biome/ipc/global_shortcuts_portal.h's GlobalShortcutSpec exactly (same
// wire shape), redeclared here since forest and biome are separate repos
// with no shared header.
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

} // namespace

GlobalShortcutsPortal::GlobalShortcutsPortal(QObject *parent) : QObject(parent) {
    const bool ok = QDBusConnection::sessionBus().connect(
        QString(kBusService), QString(kObjectPath), QString(kGlobalShortcutsIface),
        QStringLiteral("Activated"), this,
        SLOT(handleActivated(QDBusObjectPath, QString, qulonglong, QVariantMap)));
    if (!ok) {
        qWarning() << "GlobalShortcutsPortal: failed to connect to Activated signal:"
                   << QDBusConnection::sessionBus().lastError().message();
    }
}

QString GlobalShortcutsPortal::newHandleToken() {
    static QAtomicInt counter{0};
    return QStringLiteral("forest_hotkeys_%1").arg(counter.fetchAndAddRelaxed(1));
}

namespace {

// Sender name mangled the way every portal object-path convention requires
// (drop the leading ':', '.' -> '_') - shared by the Request path (derived
// from the daemon's own CreateSession/BindShortcuts return value) and the
// Session path below (which, unlike the Request path, is never handed back
// by the daemon at all - the client is required to derive it itself from
// its own sender name and the session_handle_token it chose).
QString escapedSender() {
    QString sender = QDBusConnection::sessionBus().baseService();
    sender.remove(0, 1);
    sender.replace('.', '_');
    return sender;
}

} // namespace

void GlobalShortcutsPortal::awaitResponse(const QDBusObjectPath &path,
        std::function<void(uint code, const QVariantMap &results)> then) {
    auto *request = new PortalRequest(std::move(then), this);
    const bool ok = QDBusConnection::sessionBus().connect(
        QString(kBusService), path.path(), QString(kRequestIface), QStringLiteral("Response"),
        request, SLOT(onResponse(uint, QVariantMap)));
    if (!ok) {
        qWarning() << "GlobalShortcutsPortal: failed to connect to Request::Response at" << path.path();
        request->onResponse(1, {});
    }
}

void GlobalShortcutsPortal::createSession(std::function<void(bool ok)> onReady) {
    QDBusInterface iface(kBusService, kObjectPath, kGlobalShortcutsIface, QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        qWarning() << "GlobalShortcutsPortal: GlobalShortcuts interface unavailable:"
                   << QDBusConnection::sessionBus().lastError().message();
        onReady(false);
        return;
    }

    const QString sessionToken = newHandleToken();
    QVariantMap options;
    options.insert(QStringLiteral("handle_token"), newHandleToken());
    options.insert(QStringLiteral("session_handle_token"), sessionToken);

    // Per the portal spec's session-handle convention, this path is never
    // handed back in the Response - it's derived the same way the daemon
    // itself derives it, from our own sender name and the
    // session_handle_token we just chose, so it's set here rather than
    // read out of `results` below.
    m_sessionHandle = QDBusObjectPath(
        QStringLiteral("/org/freedesktop/portal/desktop/session/%1/%2").arg(escapedSender(), sessionToken));

    const QDBusReply<QDBusObjectPath> reply = iface.call(QStringLiteral("CreateSession"), options);
    if (!reply.isValid()) {
        qWarning() << "GlobalShortcutsPortal: CreateSession call failed:" << reply.error().message();
        m_sessionHandle = QDBusObjectPath();
        onReady(false);
        return;
    }

    awaitResponse(reply.value(), [this, onReady](uint code, const QVariantMap &results) {
        Q_UNUSED(results);
        if (code != 0) {
            qWarning() << "GlobalShortcutsPortal: CreateSession request failed, response code" << code;
            m_sessionHandle = QDBusObjectPath();
            onReady(false);
            return;
        }
        onReady(true);
    });
}

void GlobalShortcutsPortal::bindShortcuts(const QList<globalhotkey *> &hotkeys, std::function<void(bool ok)> onDone) {
    qDBusRegisterMetaType<PortalShortcutSpec>();
    qDBusRegisterMetaType<QList<PortalShortcutSpec>>();

    QDBusInterface iface(kBusService, kObjectPath, kGlobalShortcutsIface, QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        qWarning() << "GlobalShortcutsPortal: GlobalShortcuts interface unavailable:"
                   << QDBusConnection::sessionBus().lastError().message();
        onDone(false);
        return;
    }

    QList<PortalShortcutSpec> shortcuts;
    for (globalhotkey *item : hotkeys) {
        const QString trigger = item->triggerString();
        if (trigger.isEmpty()) {
            continue; // triggerString() already logged why
        }
        PortalShortcutSpec spec;
        spec.id = item->id();
        spec.options.insert(QStringLiteral("description"), item->description());
        spec.options.insert(QStringLiteral("preferred_trigger"), trigger);
        shortcuts.append(spec);
    }

    QVariantMap options;
    options.insert(QStringLiteral("handle_token"), newHandleToken());

    const QDBusReply<QDBusObjectPath> reply = iface.call(QStringLiteral("BindShortcuts"), m_sessionHandle,
        QVariant::fromValue(shortcuts), QString(), options);
    if (!reply.isValid()) {
        qWarning() << "GlobalShortcutsPortal: BindShortcuts call failed:" << reply.error().message();
        onDone(false);
        return;
    }

    awaitResponse(reply.value(), [onDone](uint code, const QVariantMap &) {
        onDone(code == 0);
    });
}

void GlobalShortcutsPortal::closeSession(std::function<void()> onClosed) {
    if (!m_sessionHandle.path().isEmpty()) {
        QDBusInterface iface(kBusService, m_sessionHandle.path(), kSessionIface, QDBusConnection::sessionBus());
        if (iface.isValid()) {
            const QDBusReply<void> reply = iface.call(QStringLiteral("Close"));
            if (!reply.isValid()) {
                qWarning() << "GlobalShortcutsPortal: Session.Close() failed:" << reply.error().message();
            }
        }
        m_sessionHandle = QDBusObjectPath();
    }
    onClosed();
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

#include "globalshortcutsportal.moc"
