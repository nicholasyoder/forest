// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TOPLEVELCAPTURE_H
#define TOPLEVELCAPTURE_H

#include <QImage>
#include <QObject>
#include <QPointer>

class ExtForeignToplevelHandle;
struct wl_output;
struct ToplevelCaptureData;

// Streams one window's (or one output's) pixels via ext-image-copy-capture-v1: a frame is
// requested after each `ready`, until stop() or destruction. Works for
// hidden windows too (Biome captures a private scene). Check isSupported().
class ToplevelCapture : public QObject {
    Q_OBJECT

public:
    explicit ToplevelCapture(ExtForeignToplevelHandle *handle, QObject *parent = nullptr);
    // Whole-output source; frames are in physical pixels.
    explicit ToplevelCapture(wl_output *output, QObject *parent = nullptr);
    ~ToplevelCapture();

    // Binds the protocol globals early so isSupported() is accurate by first use.
    static void preload();
    // True when the compositor offers per-toplevel capture.
    static bool isSupported();
    // True when the compositor offers per-output capture.
    static bool isOutputSupported();

    void start();
    void stop();
    // Request the next frame; no-op if one is in flight or not started.
    void requestFrame();

signals:
    void frameReady(const QImage &image);
    // Session stopped or a capture failed; the owner should fall back.
    void failed();

private:
    ToplevelCaptureData *d;
};

#endif // TOPLEVELCAPTURE_H
