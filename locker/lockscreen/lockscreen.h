// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOCKSCREEN_H
#define LOCKSCREEN_H

#include <QHash>
#include <QObject>

#include "miscutills.h"
#include "pamauth.h"

class LockWindow;
class PasswordCard;
class QScreen;

// Locks, covers every screen, authenticates, unlocks. Exit code: 0 unlocked, 1 lock refused.
class LockScreen : public QObject {
    Q_OBJECT
public:
    explicit LockScreen(QObject *parent = nullptr);
    ~LockScreen() override;

    bool start();
    void unlockAndQuit();

private:
    void addScreen(QScreen *screen);
    void removeScreen(QScreen *screen);
    void placeCard();

    QHash<QScreen *, LockWindow *> m_windows;
    PasswordCard *m_card;
    PamAuth m_pam;
    ScreenTracker m_tracker;
};

#endif // LOCKSCREEN_H
