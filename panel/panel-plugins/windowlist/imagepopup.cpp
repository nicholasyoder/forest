// SPDX-License-Identifier: LGPL-3.0-or-later

#include "imagepopup.h"
#include <QGraphicsDropShadowEffect>

namespace { constexpr int labelMargin = 10; }

imagepopup::imagepopup(QWidget *parentw){
    parentwidget = parentw;
}

// A fresh popup per shown preview: reusing one carried the previous window's
// size and position over, so it resized or re-centered after it was visible.
void imagepopup::buildPopup(const QPixmap &pix){
    destroyPopup();

    popupglayout = new QGridLayout;
    popupglayout->setContentsMargins(QMargins(0,0,0,0));
    popupglayout->setSpacing(3);

    closebutton *cbt = new closebutton;
    cbt->setFixedSize(18,18);
    popupglayout->addWidget(cbt, 0,1);

    QGraphicsDropShadowEffect *ds_effect = new QGraphicsDropShadowEffect();
    ds_effect->setBlurRadius(18);
    ds_effect->setOffset(0,0);
    ds_effect->setColor(Qt::black);
    scrshotlabel = new QLabel;
    scrshotlabel->setAlignment(Qt::AlignCenter);
    scrshotlabel->setGraphicsEffect(ds_effect);
    // Margin leaves room for the drop shadow; setting it in the QSS instead
    // would shrink the content rect and crop the pixmap.
    scrshotlabel->setContentsMargins(labelMargin, labelMargin, labelMargin, labelMargin);
    scrshotlabel->setFixedSize(pix.deviceIndependentSize().toSize() + QSize(2 * labelMargin, 2 * labelMargin));
    scrshotlabel->setPixmap(pix);
    popupglayout->addWidget(scrshotlabel, 1,0, 1, 2);

    pbox = new popup(popupglayout, parentwidget, CenteredOnWidget, false);
    connect(cbt, &closebutton::clicked, pbox, &popup::closepopup);
    connect(cbt, &closebutton::clicked, this, &imagepopup::closewindow);
    connect(pbox, &popup::mousereleased, this, &imagepopup::raisewindow);
}

void imagepopup::destroyPopup(){
    if (!pbox) return;
    pbox->closepopup();
    pbox->deleteLater();
    pbox = nullptr;
    scrshotlabel = nullptr;
    popupglayout = nullptr;
}

void imagepopup::btmouseEnter(windowbutton *bt){
    if (!popup_enabled) return;

    currentbt = bt;

    if (!open){
        if (openptimer)
            delete openptimer;

        openptimer = new QTimer;
        openptimer->setSingleShot(true);
        connect(openptimer, &QTimer::timeout, this, &imagepopup::showpopup);
        openptimer->start(450);
    }
    else {
        beginPreview();
    }
}

void imagepopup::btmouseLeave(){
    if (!popup_enabled) return;
    deleteopenptimer();
}

void imagepopup::btclicked(){
    if (!currentbt) return;
    if (pbox && pbox->isVisible()) closepopup();
    else deleteopenptimer();
}

void imagepopup::showpopup(){
    if (!currentbt) return;

    open = true;

    if (closeptimer){
        closeptimer->stop();
        delete closeptimer;
    }
    closeptimer = new QTimer;
    connect(closeptimer, &QTimer::timeout, this, &imagepopup::tryclosepopup);
    closeptimer->start(450);

    beginPreview();
}

void imagepopup::beginPreview(){
    endCapture();
    previewShown = false;

    ExtForeignToplevelHandle *ext = currentbt->toplevelHandle()->extHandle();
    if (!ext || !ToplevelCapture::isSupported()){
        presentPixmap(get_window_image());
        return;
    }

    capture = new ToplevelCapture(ext, this);
    connect(capture, &ToplevelCapture::frameReady, this, &imagepopup::onFrame);
    connect(capture, &ToplevelCapture::failed, this, &imagepopup::onCaptureFailed);

    // ~15 fps is plenty for a thumbnail and keeps the target app's cost low.
    frameTimer = new QTimer(capture);
    frameTimer->setSingleShot(true);
    connect(frameTimer, &QTimer::timeout, capture, &ToplevelCapture::requestFrame);

    // Don't leave the popup hanging if the first frame is slow.
    firstFrameTimer = new QTimer(capture);
    firstFrameTimer->setSingleShot(true);
    connect(firstFrameTimer, &QTimer::timeout, this, [this]{
        if (!previewShown) presentPixmap(get_window_image());
    });
    firstFrameTimer->start(250);

    capture->start();
    capture->requestFrame();
}

void imagepopup::endCapture(){
    if (!capture) return;
    capture->disconnect(this);
    delete capture; // also deletes its timers
    capture = nullptr;
    firstFrameTimer = nullptr;
    frameTimer = nullptr;
}

void imagepopup::presentPixmap(const QPixmap &pix){
    if (!currentbt) return;
    // Rebuild when switching windows or when the image's proportions change.
    if (!previewShown || !scrshotlabel || scrshotlabel->pixmap().deviceIndependentSize() != pix.deviceIndependentSize()){
        buildPopup(pix);
        previewShown = true;
        pbox->changelauncher(currentbt);
        pbox->showpopup();
    } else {
        scrshotlabel->setPixmap(pix);
    }
}

void imagepopup::onFrame(const QImage &image){
    if (!currentbt || image.isNull()) return;

    const QSize maxSize(preview_width, preview_width * 5 / 8);
    const qreal dpr = parentwidget->devicePixelRatioF();
    QImage scaled = image;
    if (image.width() > maxSize.width() * dpr || image.height() > maxSize.height() * dpr)
        scaled = image.scaled((QSizeF(maxSize) * dpr).toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPixmap pix = QPixmap::fromImage(scaled);
    pix.setDevicePixelRatio(dpr);
    presentPixmap(pix);

    if (frameTimer) frameTimer->start(66);
}

void imagepopup::onCaptureFailed(){
    if (!previewShown) presentPixmap(get_window_image());
}

void imagepopup::tryclosepopup(){
    if (!currentbt){
        if (closeptimer){
            closeptimer->stop();
            delete closeptimer;
            closeptimer = nullptr;
        }
        return;
    }

    // Not QCursor::pos(): on Wayland it goes stale once the pointer leaves our surfaces.
    if (!(currentbt->underMouse() || (pbox && pbox->underMouse())))
        closepopup();
}

void imagepopup::closepopup(){
    if (!popup_enabled) return;
    open = false;

    if (closeptimer){
        closeptimer->stop();
        delete closeptimer;
        closeptimer = nullptr;
    }

    endCapture();
    previewShown = false;
    currentbt = nullptr;

    destroyPopup();
}

void imagepopup::deleteopenptimer(){
    if (openptimer){
        openptimer->stop();
        delete openptimer;
        openptimer = nullptr;
    }
}

QPixmap imagepopup::get_window_image(){
    return currentbt->icon().pixmap(120, 120);
}
