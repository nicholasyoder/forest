// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TRAYICON_H
#define TRAYICON_H

#include <QFrame>
#include <QTimer>
#include "panelbutton.h"

#include <X11/X.h>
#include <X11/extensions/Xdamage.h>

#define TRAY_ICON_SIZE_DEFAULT 24

class QWidget;
class LXQtPanel;

class TrayIcon: public panelbutton
{
    Q_OBJECT
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)

public:
    TrayIcon(Window iconId, QSize const & iconSize);
    virtual ~TrayIcon();

    Window iconId() { return mIconId; }
    Window windowId() { return mWindowId; }
    void windowDestroyed(Window w);

    QSize iconSize() const { return mIconSize; }
    void setIconSize(QSize iconSize);
    void updateicon();

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent*) override;
    void leaveEvent(QEvent *event) override;

private:
    void init();

    QRect iconGeometry();
    Window mIconId;
    Window mWindowId;
    QSize mIconSize;
    Damage mDamage;
    Display* mDisplay;
    QImage getImageNonComposite();
    QSize calculateClientWindowSize();
    QTimer *highlight_hack_timer = nullptr;
};

#endif // TRAYICON_H
