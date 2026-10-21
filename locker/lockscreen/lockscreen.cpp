// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lockscreen.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

#include <cstdio>
#include <pwd.h>
#include <unistd.h>

#include "loginui.h"
#include "passwordform.h"
#include "screenset.h"
#include "sessionlock.h"

namespace {

constexpr int NoPromptRetryMs = 2000;

} // namespace

LockScreen::LockScreen(QObject *parent)
    : QObject(parent)
    , m_form(new loginui::PasswordForm)
{
    QString username, displayName, homeDir;
    if (passwd *pw = getpwuid(getuid())) {
        username = QString::fromLocal8Bit(pw->pw_name);
        homeDir = QString::fromLocal8Bit(pw->pw_dir);
        displayName = QString::fromLocal8Bit(pw->pw_gecos).split(',').first().trimmed();
    }
    if (displayName.isEmpty())
        displayName = username;
    m_form->setUser(displayName, loginui::faceIconPath(username, homeDir));

    auto *card = new loginui::Card;
    card->addWidget(m_form);
    // Primary first: compositors tend to focus the first lock surface.
    m_screens = new loginui::ScreenSet(
        card, [](QWindow *window) { sessionlock::Lock::instance()->attach(window); },
        loginui::ScreenSet::InitialFocus::First, this);
    connect(m_screens, &loginui::ScreenSet::cardPlaced, m_form, &loginui::PasswordForm::focusInput);

    auto *lock = sessionlock::Lock::instance();
    connect(lock, &sessionlock::Lock::locked, this, [this] {
        // Readiness for forest-locker: the screens are actually covered now.
        fputs("locked\n", stdout);
        fflush(stdout);
        if (m_unlockRequested)
            unlockAndQuit();
    });
    connect(lock, &sessionlock::Lock::finished, this, [] {
        qWarning() << "Session lock refused or revoked by the compositor";
        QCoreApplication::exit(1);
    });

    connect(&m_pam, &PamAuth::prompt, m_form, &loginui::PasswordForm::setPrompt);
    connect(&m_pam, &PamAuth::message, m_form, &loginui::PasswordForm::setStatus);
    connect(&m_pam, &PamAuth::succeeded, this, &LockScreen::unlockAndQuit);
    connect(&m_pam, &PamAuth::failed, this, [this](const QString &reason) {
        m_form->setStatus(reason.isEmpty() ? "Authentication failed" : reason, true);
        // A round that fails without prompting would otherwise retry in a tight loop.
        if (m_pam.prompted())
            m_pam.start();
        else
            QTimer::singleShot(NoPromptRetryMs, &m_pam, &PamAuth::start);
    });
    connect(m_form, &loginui::PasswordForm::submitted, &m_pam, &PamAuth::respond);
}

bool LockScreen::start()
{
    if (!sessionlock::Lock::instance()->lock())
        return false;

    m_screens->showOnAllScreens();

    m_pam.start();
    return true;
}

void LockScreen::unlockAndQuit()
{
    // Before `locked`, unlock() would only drop our lock request, leaving a crashed
    // predecessor's lock in place.
    if (!sessionlock::Lock::instance()->isLocked()) {
        m_unlockRequested = true;
        return;
    }
    sessionlock::Lock::instance()->unlock();
    // Not a normal exit: ~PamAuth would wait on a module blocked outside the conversation
    // (e.g. fprintd), keeping forest-locker in its locked state.
    fflush(nullptr);
    _exit(0);
}
