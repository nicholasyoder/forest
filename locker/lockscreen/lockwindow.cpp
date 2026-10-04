// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lockwindow.h"

#include <QHBoxLayout>
#include <QScreen>
#include <QVBoxLayout>

LockWindow::LockWindow(QScreen *screen)
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
    clockLayout->addWidget(new loginui::Clock);
    clockLayout->addStretch(1);
    row->addLayout(clockLayout);
    row->addStretch(1);
    m_content->hide();

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_content);
    mainLayout->addStretch(1);
}

void LockWindow::setCard(QWidget *card)
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
