// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TRAYICON_H
#define TRAYICON_H

#include "panelbutton.h"

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>

class trayicon: public panelbutton
{
    Q_OBJECT

public:
    explicit trayicon(xcb_window_t wid);
    ~trayicon() override;

    void resizeWindow(const uint16_t width, const uint16_t height) const;
    void hideContainerWindow(xcb_window_t windowId) const;

    QString Title() const;

private slots:
    void handleClick(QMouseEvent *event);

    // Shows the context menu associated to this item at the desired screen position
    void ContextMenu(int x, int y);

    //Shows the main widget and try to position it on top of the other windows, if the widget is already visible, hide it.
    void Activate(int x, int y);

    // The user activated the item in an alternate way (for instance with middle mouse button, this depends from the systray implementation)
    void SecondaryActivate(int x, int y);

    // Inform this item that the mouse wheel was used on its representation
    void Scroll(int delta, const QString &orientation);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override;
    //void mouseMoveEvent(QMouseEvent *event) override;

private:
    enum InjectMode {
        Direct,
        XTest
    };

    void init();
    void updateIcon();

    QSize calculateClientWindowSize() const;
    void sendClick(uint8_t mouseButton, int x, int y);
    QImage getImageNonComposite() const;
    bool isTransparentImage(const QImage &image) const;
    QImage convertFromNative(xcb_image_t *xcbImage) const;
    QPoint calculateClickPoint() const;
    void stackContainerWindow(const uint32_t stackMode) const;

    xcb_window_t iWindowId;
    xcb_window_t iContainerWid;

    QPixmap iPixmap;
    bool sendingClickEvent;
    InjectMode iInjectMode;

};

#endif // TRAYICON_H
