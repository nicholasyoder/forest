// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SESSIONLOCK_H
#define SESSIONLOCK_H

#include <QObject>

#include <memory>

class QWindow;

namespace sessionlock {

class Manager;
class LockObject;

// ext-session-lock-v1 client: lock(), then attach() one window per screen.
class Lock : public QObject {
    Q_OBJECT
public:
    static Lock *instance();

    // False if the compositor lacks ext-session-lock-v1.
    bool lock();
    // Returns once the compositor has processed the unlock, so exiting right after is safe.
    void unlock();
    bool isLocked() const { return m_locked; }

    // Makes `window` a lock surface on its screen(). Call before show(); never hide it
    // while locked (that destroys the lock surface and the compositor blanks the output).
    void attach(QWindow *window);

signals:
    void locked();
    // Lock refused (another locker holds it) or revoked by the compositor.
    void finished();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Lock();
    ~Lock() override;
    void setIntegration(QWindow *window);
    void releaseLock();

    Manager *m_manager = nullptr;
    std::unique_ptr<LockObject> m_lock;
    bool m_locked = false;

    friend class Manager;
    friend class LockObject;
};

} // namespace sessionlock

#endif // SESSIONLOCK_H
