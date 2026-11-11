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
#include "toplevelcapture.h"

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
    void set_preview_width(int width){ preview_width = width; }

    void closepopup();

private slots:
    void showpopup();
    void tryclosepopup();

    void deleteopenptimer();
    void closewindow(){if(currentbt){ currentbt->toplevelHandle()->requestClose(); closepopup();}}
    void raisewindow(){if(currentbt){ currentbt->toplevelHandle()->activate(); closepopup();}}

private:
    // Icon placeholder, used until (or instead of) a live frame.
    QPixmap get_window_image();

    // Starts a live capture of currentbt's window and shows the popup on the
    // first frame (or with the icon after a short wait / if unsupported).
    void buildPopup(const QPixmap &pix);
    void destroyPopup();
    void beginPreview();
    void endCapture();
    void presentPixmap(const QPixmap &pix);
    void onFrame(const QImage &image);
    void onCaptureFailed();

    bool popup_enabled = true;
    int preview_width = 250; // max logical width; height is capped at 5/8 of it

    QPointer<windowbutton> currentbt;
    bool open = false;
    bool previewShown = false; // for the current button

    ToplevelCapture *capture = nullptr;
    QTimer *firstFrameTimer = nullptr;
    QTimer *frameTimer = nullptr;

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
