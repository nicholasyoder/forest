// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCREENGRAB_H
#define SCREENGRAB_H

#include <QImage>
#include <QObject>

class ExtForeignToplevelHandle;
class QScreen;
class QTimer;
class ToplevelCapture;

// One-shot grab of a window or an output via ToplevelCapture. Emits finished()
// exactly once (null image on failure or timeout), then deletes itself.
class ScreenGrab : public QObject {
    Q_OBJECT

public:
    // Physical pixels (not downscaled). Both return a grab already started.
    static ScreenGrab *grabOutput(QScreen *screen, QObject *parent = nullptr);
    static ScreenGrab *grabWindow(ExtForeignToplevelHandle *handle, QObject *parent = nullptr);

signals:
    void finished(const QImage &image);

private:
    explicit ScreenGrab(QObject *parent);
    void begin(ToplevelCapture *capture);
    void finish(const QImage &image);

    ToplevelCapture *m_capture = nullptr;
    QTimer *m_timeout = nullptr;
    bool m_done = false;
};

#endif // SCREENGRAB_H
