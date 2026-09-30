// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef IMAGEPOPUP_H
#define IMAGEPOPUP_H

#include <QLabel>
#include <QGridLayout>
#include <QPointer>
#include <QTimer>

#include "popup.h"
#include "windowbutton.h"
#include "closebutton.h"

class imagepopup : public QObject
{
    Q_OBJECT
public:
    imagepopup(QWidget *parentw);

signals:

public slots:
    void btmouseEnter(windowbutton *bt);
    void btmouseLeave();
    void btclicked();
    void set_enabled(bool enabled = true){ popup_enabled = enabled; }

    void closepopup();

private slots:
    void showpopup();
    void tryclosepopup();

    void deleteopenptimer();
    void closewindow(){if(currentbt){ currentbt->toplevelHandle()->requestClose(); closepopup();}}
    void raisewindow(){if(currentbt){ currentbt->toplevelHandle()->activate(); closepopup();}}

private:
    // Icon-only: no protocol here captures an arbitrary window's pixels
    // (would need ext-image-copy-capture / wlr-screencopy).
    QPixmap get_window_image();

    bool popup_enabled = true;

    QPointer<windowbutton> currentbt;
    bool open = false;

    QTimer *openptimer = nullptr;
    QTimer *closeptimer = nullptr;
    popup *pbox = nullptr;
    QLabel *scrshotlabel = nullptr;
    QLabel *wintitlelabel = nullptr;
    //QVBoxLayout *popupvlayout = nullptr;
    QGridLayout *popupglayout = nullptr;
    QWidget *parentwidget = nullptr;
};

#endif // IMAGEPOPUP_H
