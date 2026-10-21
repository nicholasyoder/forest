// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCREENSET_H
#define SCREENSET_H

#include <QHash>
#include <QObject>

#include <functional>

#include "loginui.h"

class QScreen;
class QVBoxLayout;
class QWindow;
class ScreenTracker;

namespace loginui {

// One screen's surface: wallpaper, plus the card and clock when it hosts the card.
class ScreenWindow : public ScreenBackground {
    Q_OBJECT
public:
    explicit ScreenWindow(QScreen *screen);

    // Takes ownership; nullptr detaches the current card (the caller re-hosts it).
    void setCard(QWidget *card);
    QWidget *card() const { return m_card; }

private:
    QWidget *m_content;
    QVBoxLayout *m_cardLayout;
    QWidget *m_card = nullptr;
};

// A ScreenWindow per screen, with the card on the focused one.
class ScreenSet : public QObject {
    Q_OBJECT
public:
    // Which window the compositor is expected to focus initially; that one gets the primary screen.
    enum class InitialFocus { First, Last };

    // setupSurface runs on each window's QWindow before it is shown.
    ScreenSet(QWidget *card, std::function<void(QWindow *)> setupSurface, InitialFocus focus,
              QObject *parent = nullptr);
    ~ScreenSet() override;

    void showOnAllScreens();

signals:
    // The card moved to another window; focus its input again.
    void cardPlaced();

private:
    void addScreen(QScreen *screen);
    void removeScreen(QScreen *screen);
    void placeCard();

    QWidget *m_card;
    std::function<void(QWindow *)> m_setupSurface;
    InitialFocus m_focus;
    QHash<QScreen *, ScreenWindow *> m_windows;
    ScreenTracker *m_tracker;
};

} // namespace loginui

#endif // SCREENSET_H
