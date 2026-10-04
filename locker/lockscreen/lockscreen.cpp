// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lockscreen.h"

#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

#include <cstdio>

#include "lockwindow.h"
#include "passwordcard.h"
#include "sessionlock.h"

LockScreen::LockScreen(QObject *parent)
    : QObject(parent)
    , m_card(new PasswordCard)
{
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

    connect(&m_pam, &PamAuth::prompt, m_card, &PasswordCard::setPrompt);
    connect(&m_pam, &PamAuth::message, m_card, &PasswordCard::setStatus);
    connect(&m_pam, &PamAuth::succeeded, this, &LockScreen::unlockAndQuit);
    connect(&m_pam, &PamAuth::failed, this, [this](const QString &reason) {
        m_card->setStatus(reason.isEmpty() ? "Authentication failed" : reason, true);
        m_pam.start();
    });
    connect(m_card, &PasswordCard::submitted, &m_pam, &PamAuth::respond);

    connect(qGuiApp, &QGuiApplication::screenAdded, this, &LockScreen::addScreen);
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, &LockScreen::removeScreen);
    connect(&m_tracker, &ScreenTracker::screens_replaced, this, &LockScreen::placeCard);
    // The compositor picks which lock surface gets the keyboard; put the card there.
    connect(qGuiApp, &QGuiApplication::focusWindowChanged, this, &LockScreen::placeCard);
}

LockScreen::~LockScreen()
{
    // Not owned by a window while detached.
    if (!m_card->parent())
        delete m_card;
}

bool LockScreen::start()
{
    if (!sessionlock::Lock::instance()->lock())
        return false;

    // Primary first: compositors tend to focus the first lock surface.
    QScreen *primary = ScreenTracker::primary();
    addScreen(primary);
    for (QScreen *screen : QGuiApplication::screens())
        addScreen(screen);

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
    QCoreApplication::exit(0);
}

void LockScreen::addScreen(QScreen *screen)
{
    if (!screen || m_windows.contains(screen))
        return;
    auto *window = new LockWindow(screen);
    window->winId();
    sessionlock::Lock::instance()->attach(window->windowHandle());
    m_windows.insert(screen, window);
    placeCard();
    window->show();
}

void LockScreen::removeScreen(QScreen *screen)
{
    LockWindow *window = m_windows.take(screen);
    if (!window)
        return;
    window->setCard(nullptr);
    delete window;
    placeCard();
}

void LockScreen::placeCard()
{
    // The focused window, else wherever the card already is, else primary, else any.
    LockWindow *host = nullptr;
    for (LockWindow *window : std::as_const(m_windows)) {
        if (window->windowHandle() == QGuiApplication::focusWindow())
            host = window;
        else if (!host && window->card())
            host = window;
    }
    if (!host)
        host = m_windows.value(ScreenTracker::primary());
    if (!host && !m_windows.isEmpty())
        host = *m_windows.cbegin();
    if (!host || host->card() == m_card)
        return;
    for (LockWindow *window : std::as_const(m_windows)) {
        if (window->card())
            window->setCard(nullptr);
    }
    host->setCard(m_card);
    m_card->focusInput();
}
