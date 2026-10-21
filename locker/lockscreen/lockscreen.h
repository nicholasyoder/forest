// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOCKSCREEN_H
#define LOCKSCREEN_H

#include <QObject>

#include "pamauth.h"

namespace loginui {
class PasswordForm;
class ScreenSet;
}

// Locks, covers every screen, authenticates, unlocks. Exit code: 0 unlocked, 1 lock refused
// (forest-locker respawns on anything else). SIGUSR1 unlocks without authenticating.
class LockScreen : public QObject {
    Q_OBJECT
public:
    explicit LockScreen(QObject *parent = nullptr);

    bool start();
    void unlockAndQuit();

private:
    loginui::PasswordForm *m_form;
    loginui::ScreenSet *m_screens;
    PamAuth m_pam;
    bool m_unlockRequested = false;
};

#endif // LOCKSCREEN_H
