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
    //void resizepbox(){pbox->resize(pbox->sizeHint()); pbox->positionOnLauncher();}

private:
    // No wlr-foreign-toplevel-management equivalent exists for capturing an
    // arbitrary (possibly minimized/off-screen) client window's pixels -
    // only compositor-side output capture does. Icon-only for now; a live
    // wlr-screencopy capture of currently-mapped windows is a possible
    // later upgrade - tracked as the screenshots item in biome/docs/roadmap.md's
    // Phase 6.
    // The popup/timer/positioning/shadow machinery below is kept as-is so
    // that upgrade has somewhere to plug back in.
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
