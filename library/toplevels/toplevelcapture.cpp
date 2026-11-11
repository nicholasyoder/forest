// SPDX-License-Identifier: LGPL-3.0-or-later

#include "toplevelcapture.h"

#include "extforeigntoplevelhandle.h"

#include <QWaylandClientExtension>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include <QtWaylandClient/private/qwaylandshm_p.h>

#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>

#include "qwayland-ext-image-capture-source-v1.h"
#include "qwayland-ext-image-copy-capture-v1.h"

namespace {

class SourceManager : public QWaylandClientExtensionTemplate<SourceManager>,
                      public QtWayland::ext_foreign_toplevel_image_capture_source_manager_v1 {
public:
    SourceManager() : QWaylandClientExtensionTemplate<SourceManager>(1) {}
};

class CopyManager : public QWaylandClientExtensionTemplate<CopyManager>,
                    public QtWayland::ext_image_copy_capture_manager_v1 {
public:
    CopyManager() : QWaylandClientExtensionTemplate<CopyManager>(1) {}
};

// Created once the QApplication exists; they bind on the first event-loop pass.
SourceManager *sourceManager() {
    static SourceManager *m = new SourceManager;
    return m;
}

CopyManager *copyManager() {
    static CopyManager *m = new CopyManager;
    return m;
}

constexpr uint32_t WL_SHM_ARGB8888 = 0;
constexpr uint32_t WL_SHM_XRGB8888 = 1;
// wlroots may send the DRM fourcc instead of the wl_shm enum for ARGB/XRGB;
// its GL renderer natively offers only the ABGR/XBGR variants.
constexpr uint32_t FOURCC_ARGB8888 = 0x34325241;
constexpr uint32_t FOURCC_XRGB8888 = 0x34325258;
constexpr uint32_t FOURCC_ABGR8888 = 0x34324241;
constexpr uint32_t FOURCC_XBGR8888 = 0x34324258;

// Normalizes to the wl_shm format value; 0 if unsupported. Higher is preferred
// (alpha first, so shaped/translucent windows keep it).
int shmRank(uint32_t format, uint32_t *wlFormat) {
    switch (format) {
    case WL_SHM_ARGB8888: case FOURCC_ARGB8888: *wlFormat = WL_SHM_ARGB8888; return 4;
    case FOURCC_ABGR8888: *wlFormat = FOURCC_ABGR8888; return 3;
    case WL_SHM_XRGB8888: case FOURCC_XRGB8888: *wlFormat = WL_SHM_XRGB8888; return 2;
    case FOURCC_XBGR8888: *wlFormat = FOURCC_XBGR8888; return 1;
    }
    return 0;
}

QImage::Format imageFormat(uint32_t wlFormat) {
    switch (wlFormat) {
    case WL_SHM_ARGB8888: return QImage::Format_ARGB32_Premultiplied;
    case FOURCC_ABGR8888: return QImage::Format_RGBA8888_Premultiplied;
    case WL_SHM_XRGB8888: return QImage::Format_RGB32;
    default: return QImage::Format_RGBX8888;
    }
}

} // namespace

class CaptureSession;
class CaptureFrame;

struct ToplevelCaptureData {
    ToplevelCapture *q = nullptr;
    QPointer<ExtForeignToplevelHandle> handle;
    CaptureSession *session = nullptr;
    CaptureFrame *frame = nullptr;

    // Buffer constraints from the session, applied on `done`.
    int pendingWidth = 0;
    int pendingHeight = 0;
    int pendingFormat = -1;

    bool started = false;
    bool constraintsDone = false;
    bool wantFrame = false;

    wl_buffer *buffer = nullptr;
    uchar *map = nullptr;
    size_t mapSize = 0;
    int width = 0;
    int height = 0;
    int format = -1;

    void freeBuffer();
    bool allocateBuffer();
    void fail();
    void issueFrame();
};

class CaptureSession : public QtWayland::ext_image_copy_capture_session_v1 {
public:
    CaptureSession(::ext_image_copy_capture_session_v1 *object, ToplevelCaptureData *data)
        : QtWayland::ext_image_copy_capture_session_v1(object), d(data) {}
    ~CaptureSession() override { destroy(); }

protected:
    void ext_image_copy_capture_session_v1_buffer_size(uint32_t width, uint32_t height) override {
        d->pendingWidth = int(width);
        d->pendingHeight = int(height);
        d->pendingFormat = -1;
    }
    void ext_image_copy_capture_session_v1_shm_format(uint32_t format) override {
        uint32_t wlFormat = 0;
        int rank = shmRank(format, &wlFormat);
        uint32_t best = 0;
        if (rank > 0 && (d->pendingFormat < 0 || rank > shmRank(uint32_t(d->pendingFormat), &best)))
            d->pendingFormat = int(wlFormat);
    }
    void ext_image_copy_capture_session_v1_done() override {
        if (d->pendingFormat < 0 || d->pendingWidth <= 0 || d->pendingHeight <= 0) {
            d->fail();
            return;
        }
        if (d->buffer && d->width == d->pendingWidth && d->height == d->pendingHeight && d->format == d->pendingFormat) {
            d->constraintsDone = true;
        } else {
            // A frame in flight holds the old buffer; drop it before freeing.
            delete d->frame;
            d->frame = nullptr;
            d->freeBuffer();
            d->width = d->pendingWidth;
            d->height = d->pendingHeight;
            d->format = d->pendingFormat;
            if (!d->allocateBuffer()) {
                d->fail();
                return;
            }
            d->constraintsDone = true;
        }
        if (d->wantFrame)
            d->issueFrame();
    }
    void ext_image_copy_capture_session_v1_stopped() override { d->fail(); }

private:
    ToplevelCaptureData *d;
};

class CaptureFrame : public QtWayland::ext_image_copy_capture_frame_v1 {
public:
    CaptureFrame(::ext_image_copy_capture_frame_v1 *object, ToplevelCaptureData *data)
        : QtWayland::ext_image_copy_capture_frame_v1(object), d(data) {}
    ~CaptureFrame() override { destroy(); }

protected:
    void ext_image_copy_capture_frame_v1_ready() override {
        QImage::Format fmt = imageFormat(uint32_t(d->format));
        // copy(): the buffer is reused for the next frame.
        QImage image = QImage(d->map, d->width, d->height, d->width * 4, fmt).copy();
        ToplevelCaptureData *data = d;
        delete data->frame; // `this`; nothing below touches members
        data->frame = nullptr;
        emit data->q->frameReady(image);
    }
    void ext_image_copy_capture_frame_v1_failed(uint32_t reason) override {
        ToplevelCaptureData *data = d;
        delete data->frame;
        data->frame = nullptr;
        // buffer_constraints: fresh constraints + `done` follow, which re-issue the frame.
        if (reason == failure_reason_buffer_constraints) {
            data->wantFrame = true;
            data->constraintsDone = false;
            return;
        }
        data->fail();
    }

private:
    ToplevelCaptureData *d;
};

void ToplevelCaptureData::freeBuffer() {
    if (buffer)
        wl_buffer_destroy(buffer);
    if (map)
        munmap(map, mapSize);
    buffer = nullptr;
    map = nullptr;
    mapSize = 0;
}

bool ToplevelCaptureData::allocateBuffer() {
    auto *display = QtWaylandClient::QWaylandIntegration::instance()->display();
    if (!display || !display->shm())
        return false;

    const int stride = width * 4;
    mapSize = size_t(stride) * size_t(height);
    int fd = memfd_create("forest-capture", MFD_CLOEXEC);
    if (fd < 0 || ftruncate(fd, off_t(mapSize)) < 0) {
        if (fd >= 0)
            close(fd);
        return false;
    }
    void *m = mmap(nullptr, mapSize, PROT_READ, MAP_SHARED, fd, 0);
    if (m == MAP_FAILED) {
        close(fd);
        return false;
    }
    map = static_cast<uchar *>(m);
    wl_shm_pool *pool = wl_shm_create_pool(display->shm()->object(), fd, int(mapSize));
    buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, uint32_t(format));
    wl_shm_pool_destroy(pool);
    close(fd);
    return true;
}

void ToplevelCaptureData::issueFrame() {
    if (frame || !session || !constraintsDone || !buffer)
        return;
    wantFrame = false;
    frame = new CaptureFrame(session->create_frame(), this);
    frame->attach_buffer(buffer);
    frame->damage_buffer(0, 0, width, height);
    frame->capture();
}

void ToplevelCaptureData::fail() {
    q->stop();
    emit q->failed();
}

ToplevelCapture::ToplevelCapture(ExtForeignToplevelHandle *handle, QObject *parent)
    : QObject(parent), d(new ToplevelCaptureData) {
    d->q = this;
    d->handle = handle;
}

ToplevelCapture::~ToplevelCapture() {
    stop();
    delete d;
}

void ToplevelCapture::preload() {
    sourceManager();
    copyManager();
}

bool ToplevelCapture::isSupported() {
    return sourceManager()->isActive() && copyManager()->isActive();
}

void ToplevelCapture::start() {
    if (d->started || !d->handle || !isSupported())
        return;
    ::ext_image_capture_source_v1 *source = sourceManager()->create_source(d->handle->object());
    if (!source) {
        emit failed();
        return;
    }
    d->started = true;
    d->session = new CaptureSession(copyManager()->create_session(source, 0), d);
    // The session keeps the source alive.
    ext_image_capture_source_v1_destroy(source);
}

void ToplevelCapture::stop() {
    d->started = false;
    d->constraintsDone = false;
    d->wantFrame = false;
    delete d->frame; // before the buffer it references
    d->frame = nullptr;
    delete d->session;
    d->session = nullptr;
    d->freeBuffer();
    d->width = d->height = 0;
    d->format = -1;
}

void ToplevelCapture::requestFrame() {
    if (!d->started)
        return;
    d->wantFrame = true;
    d->issueFrame();
}
