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
    connect(lock, &sessionlock::Lock::locked, this, [] {
        // Readiness for forest-locker: the screens are actually covered now.
        fputs("locked\n", stdout);
        fflush(stdout);
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

    // Primary first: Biome gives keyboard focus to the first lock surface.
    QScreen *primary = ScreenTracker::primary();
    addScreen(primary);
    for (QScreen *screen : QGuiApplication::screens())
        addScreen(screen);

    m_pam.start();
    return true;
}

void LockScreen::unlockAndQuit()
{
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
    LockWindow *host = m_windows.value(ScreenTracker::primary());
    if (!host && !m_windows.isEmpty())
        host = *m_windows.cbegin();
    for (LockWindow *window : std::as_const(m_windows)) {
        if (window != host && window->card())
            window->setCard(nullptr);
    }
    if (host)
        host->setCard(m_card);
}
