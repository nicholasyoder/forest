// SPDX-License-Identifier: LGPL-3.0-or-later

#include "screenset.h"

#include <QGuiApplication>
#include <QHBoxLayout>
#include <QScreen>
#include <QVBoxLayout>

#include "miscutills.h"

namespace loginui {

ScreenWindow::ScreenWindow(QScreen *screen)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setScreen(screen);
    setGeometry(screen->geometry());

    // Card and clock together; other screens show only the wallpaper.
    m_content = new QWidget;
    auto *row = new QHBoxLayout(m_content);
    row->addStretch(1);
    m_cardLayout = new QVBoxLayout;
    row->addLayout(m_cardLayout);
    row->addStretch(1);
    auto *clockLayout = new QVBoxLayout;
    clockLayout->addStretch(1);
    clockLayout->addWidget(new Clock);
    clockLayout->addStretch(1);
    row->addLayout(clockLayout);
    row->addStretch(1);
    m_content->hide();

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_content);
    mainLayout->addStretch(1);
}

void ScreenWindow::setCard(QWidget *card)
{
    if (m_card == card)
        return;
    if (m_card) {
        m_cardLayout->removeWidget(m_card);
        m_card->setParent(nullptr);
    }
    m_card = card;
    if (m_card) {
        m_cardLayout->addWidget(m_card);
        m_card->show();
    }
    m_content->setVisible(m_card);
}

ScreenSet::ScreenSet(QWidget *card, std::function<void(QWindow *)> setupSurface, InitialFocus focus,
                     QObject *parent)
    : QObject(parent)
    , m_card(card)
    , m_setupSurface(std::move(setupSurface))
    , m_focus(focus)
    , m_tracker(new ScreenTracker(this))
{
    connect(qGuiApp, &QGuiApplication::screenAdded, this, &ScreenSet::addScreen);
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, &ScreenSet::removeScreen);
    connect(m_tracker, &ScreenTracker::screens_replaced, this, &ScreenSet::placeCard);
    // The compositor picks which surface gets the keyboard; put the card there.
    connect(qGuiApp, &QGuiApplication::focusWindowChanged, this, &ScreenSet::placeCard);
}

ScreenSet::~ScreenSet()
{
    // Not owned by a window while detached.
    if (!m_card->parent())
        delete m_card;
}

void ScreenSet::showOnAllScreens()
{
    QScreen *primary = ScreenTracker::primary();
    if (m_focus == InitialFocus::First)
        addScreen(primary);
    for (QScreen *screen : QGuiApplication::screens()) {
        if (screen != primary)
            addScreen(screen);
    }
    addScreen(primary);
}

void ScreenSet::addScreen(QScreen *screen)
{
    if (!screen || m_windows.contains(screen))
        return;
    auto *window = new ScreenWindow(screen);
    window->winId();
    m_setupSurface(window->windowHandle());
    m_windows.insert(screen, window);
    placeCard();
    window->show();
}

void ScreenSet::removeScreen(QScreen *screen)
{
    ScreenWindow *window = m_windows.take(screen);
    if (!window)
        return;
    window->setCard(nullptr);
    delete window;
    placeCard();
}

void ScreenSet::placeCard()
{
    // The focused window, else wherever the card already is, else primary, else any.
    ScreenWindow *host = nullptr;
    for (ScreenWindow *window : std::as_const(m_windows)) {
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
    for (ScreenWindow *window : std::as_const(m_windows)) {
        if (window->card())
            window->setCard(nullptr);
    }
    host->setCard(m_card);
    emit cardPlaced();
}

} // namespace loginui
