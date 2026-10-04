// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOCKWINDOW_H
#define LOCKWINDOW_H

#include "loginui.h"

class QScreen;
class QVBoxLayout;

// One lock surface: wallpaper, plus the card and clock on the primary screen.
class LockWindow : public loginui::ScreenBackground {
    Q_OBJECT
public:
    explicit LockWindow(QScreen *screen);

    // Takes ownership; nullptr detaches the current card (the caller re-hosts it).
    void setCard(QWidget *card);
    QWidget *card() const { return m_card; }

private:
    QWidget *m_content;
    QVBoxLayout *m_cardLayout;
    QWidget *m_card = nullptr;
};

#endif // LOCKWINDOW_H
