// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionlock.h"

#include <QDebug>
#include <QGuiApplication>
#include <QPlatformSurfaceEvent>
#include <QScreen>
#include <QWindow>

#include <QtWaylandClient/private/qwaylandscreen_p.h>
#include <QtWaylandClient/private/qwaylandshellintegration_p.h>
#include <QtWaylandClient/private/qwaylandshellsurface_p.h>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

#include <wayland-client.h>

#include <functional>

#include "qwayland-ext-session-lock-v1.h"

using namespace QtWaylandClient;

namespace sessionlock {

class LockObject : public QtWayland::ext_session_lock_v1 {
public:
    LockObject(Lock *owner, ::ext_session_lock_v1 *object)
        : QtWayland::ext_session_lock_v1(object), m_owner(owner) {}

protected:
    void ext_session_lock_v1_locked() override
    {
        m_owner->m_locked = true;
        emit m_owner->locked();
    }

    void ext_session_lock_v1_finished() override
    {
        m_owner->releaseLock();
        emit m_owner->finished();
    }

private:
    Lock *m_owner;
};

class Surface : public QWaylandShellSurface, public QtWayland::ext_session_lock_surface_v1 {
public:
    Surface(QWaylandWindow *window, std::function<::ext_session_lock_surface_v1 *()> getRole)
        : QWaylandShellSurface(window)
    {
        // Qt's initWindow() ends with an empty commit, which is fatal on a lock surface:
        // take the role only after it has gone out.
        QMetaObject::invokeMethod(this, [this, getRole] {
            if (auto *object = getRole())
                init(object);
        }, Qt::QueuedConnection);
    }
    ~Surface() override
    {
        if (isInitialized())
            destroy();
    }

    // Committing before the first ack is a protocol error, so stay unexposed until then.
    bool isExposed() const override { return m_configured; }

    void applyConfigure() override
    {
        resizeFromApplyConfigure(m_pendingSize);
        // Ack only once the resize is applied: a commit at the old size after acking is a protocol error.
        if (m_ackPending) {
            ack_configure(m_pendingSerial);
            m_ackPending = false;
        }
    }

protected:
    void ext_session_lock_surface_v1_configure(uint32_t serial, uint32_t width, uint32_t height) override
    {
        m_pendingSerial = serial;
        m_pendingSize = QSize(width, height);
        m_ackPending = true;
        if (!m_configured) {
            m_configured = true;
            applyConfigure();
            window()->sendRecursiveExposeEvent();
        } else {
            applyConfigureWhenPossible();
        }
    }

private:
    QSize m_pendingSize;
    uint32_t m_pendingSerial = 0;
    bool m_ackPending = false;
    bool m_configured = false;
};

class Manager : public QWaylandShellIntegrationTemplate<Manager>, public QtWayland::ext_session_lock_manager_v1 {
public:
    Manager() : QWaylandShellIntegrationTemplate<Manager>(1) {}

    QWaylandShellSurface *createShellSurface(QWaylandWindow *window) override
    {
        LockObject *lock = Lock::instance()->m_lock.get();
        if (!lock || !lock->object()) {
            qWarning() << "sessionlock: window shown without an active lock";
            return nullptr;
        }
        auto *screen = dynamic_cast<QWaylandScreen *>(window->window()->screen()->handle());
        if (!screen) {
            qWarning() << "sessionlock: window is on a placeholder screen";
            return nullptr;
        }
        ::wl_surface *surface = window->waylandSurface()->object();
        ::wl_output *output = screen->output();
        return new Surface(window, [surface, output]() -> ::ext_session_lock_surface_v1 * {
            LockObject *lock = Lock::instance()->m_lock.get();
            return lock && lock->object() ? lock->get_lock_surface(surface, output) : nullptr;
        });
    }
};

Lock *Lock::instance()
{
    // Leaked on purpose: Wayland objects must not outlive the display at exit.
    static Lock *s_instance = new Lock;
    return s_instance;
}

Lock::Lock() = default;
Lock::~Lock() = default;

bool Lock::lock()
{
    if (m_lock && m_lock->object())
        return true;
    if (!m_manager) {
        auto *manager = new Manager;
        if (!manager->initialize(nullptr)) {
            delete manager;
            qWarning() << "sessionlock: compositor lacks ext-session-lock-v1";
            return false;
        }
        m_manager = manager;
    }
    m_lock = std::make_unique<LockObject>(this, m_manager->lock());
    return true;
}

void Lock::unlock()
{
    if (!m_lock || !m_lock->object())
        return;
    releaseLock();
    // The protocol requires a sync here, or exiting can race the unlock and leave the session locked.
    auto *app = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (app && app->display())
        wl_display_roundtrip(app->display());
}

void Lock::releaseLock()
{
    if (!m_lock || !m_lock->object())
        return;
    if (m_locked)
        m_lock->unlock_and_destroy();
    else
        m_lock->destroy();
    m_locked = false;
}

void Lock::attach(QWindow *window)
{
    window->installEventFilter(this);
    if (window->handle())
        setIntegration(window);
}

bool Lock::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::PlatformSurface
        && static_cast<QPlatformSurfaceEvent *>(event)->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated)
        setIntegration(static_cast<QWindow *>(watched));
    return false;
}

void Lock::setIntegration(QWindow *window)
{
    if (!m_manager) {
        qWarning() << "sessionlock: attach() before lock()";
        return;
    }
    if (auto *waylandWindow = dynamic_cast<QWaylandWindow *>(window->handle()))
        waylandWindow->setShellIntegration(m_manager);
}

} // namespace sessionlock
