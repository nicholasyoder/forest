// SPDX-License-Identifier: LGPL-3.0-or-later

#include "screengrab.h"

#include "toplevelcapture.h"

#include <QGuiApplication>
#include <QScreen>
#include <qpa/qplatformscreen.h>
#include <QTimer>
#include <QtWaylandClient/private/qwaylandscreen_p.h>

ScreenGrab::ScreenGrab(QObject *parent) : QObject(parent) {}

ScreenGrab *ScreenGrab::grabOutput(QScreen *screen, QObject *parent) {
    auto *grab = new ScreenGrab(parent);
    auto *wlScreen = screen ? static_cast<QtWaylandClient::QWaylandScreen *>(screen->handle()) : nullptr;
    wl_output *output = wlScreen ? wlScreen->output() : nullptr;
    if (!output || !ToplevelCapture::isOutputSupported()) {
        QTimer::singleShot(0, grab, [grab] { grab->finish({}); });
        return grab;
    }
    grab->begin(new ToplevelCapture(output, grab));
    return grab;
}

ScreenGrab *ScreenGrab::grabWindow(ExtForeignToplevelHandle *handle, QObject *parent) {
    auto *grab = new ScreenGrab(parent);
    if (!handle || !ToplevelCapture::isSupported()) {
        QTimer::singleShot(0, grab, [grab] { grab->finish({}); });
        return grab;
    }
    grab->begin(new ToplevelCapture(handle, grab));
    return grab;
}

void ScreenGrab::begin(ToplevelCapture *capture) {
    m_capture = capture;
    connect(capture, &ToplevelCapture::frameReady, this, &ScreenGrab::finish);
    connect(capture, &ToplevelCapture::failed, this, [this] { finish({}); });
    m_timeout = new QTimer(this);
    m_timeout->setSingleShot(true);
    connect(m_timeout, &QTimer::timeout, this, [this] { finish({}); });
    m_timeout->start(5000);
    capture->start();
    capture->requestFrame();
}

void ScreenGrab::finish(const QImage &image) {
    if (m_done)
        return;
    m_done = true;
    if (m_capture)
        m_capture->stop();
    emit finished(image);
    deleteLater();
}
