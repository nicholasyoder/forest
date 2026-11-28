// SPDX-License-Identifier: LGPL-3.0-or-later

#include "fadeoverlay.h"

#include <QGuiApplication>
#include <QTemporaryFile>
#include <QWindow>
#include <QWaylandClientExtension>
#include <qguiapplication_platform.h>

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandshm_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

#include <wayland-client.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "qwayland-alpha-modifier-v1.h"
#include "qwayland-viewporter.h"

namespace wallpaper {

namespace {

class AlphaModifier : public QWaylandClientExtensionTemplate<AlphaModifier>,
                      public QtWayland::wp_alpha_modifier_v1 {
public:
    AlphaModifier() : QWaylandClientExtensionTemplate<AlphaModifier>(1) { initialize(); }
};

class Viewporter : public QWaylandClientExtensionTemplate<Viewporter>,
                   public QtWayland::wp_viewporter {
public:
    Viewporter() : QWaylandClientExtensionTemplate<Viewporter>(1) { initialize(); }
};

template <class T>
T &global()
{
    static T *instance = new T;
    return *instance;
}

::wl_subcompositor *getSubcompositor(QtWaylandClient::QWaylandDisplay *display)
{
    static ::wl_subcompositor *subcomp = nullptr;
    if (subcomp || !display)
        return subcomp;
    for (const auto &g : display->globals()) {
        if (g.interface == QLatin1String("wl_subcompositor")) {
            subcomp = static_cast<::wl_subcompositor *>(
                wl_registry_bind(g.registry, g.id, &wl_subcompositor_interface, 1));
            break;
        }
    }
    return subcomp;
}

uint32_t scale32(qreal unit)
{
    return static_cast<uint32_t>(std::lround(std::clamp(unit, 0.0, 1.0) * 0xFFFFFFFFu));
}

::wl_buffer *createShmBuffer(::wl_shm *shm, const QImage &image)
{
    const QImage rgb = image.format() == QImage::Format_RGB32 ? image : image.convertToFormat(QImage::Format_RGB32);
    const int width = rgb.width();
    const int height = rgb.height();
    const int stride = rgb.bytesPerLine();
    const size_t size = size_t(stride) * size_t(height);
    if (size == 0)
        return nullptr;

    int fd = -1;
#ifdef SYS_memfd_create
    fd = static_cast<int>(syscall(SYS_memfd_create, "forest-wallpaper-fade", MFD_CLOEXEC));
#endif
    QTemporaryFile fallback;
    if (fd < 0) {
        if (!fallback.open())
            return nullptr;
        fd = dup(fallback.handle());
        if (fd < 0)
            return nullptr;
    }

    if (ftruncate(fd, off_t(size)) < 0) {
        close(fd);
        return nullptr;
    }
    void *mapped = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        close(fd);
        return nullptr;
    }
    std::memcpy(mapped, rgb.constBits(), size);
    munmap(mapped, size);

    ::wl_shm_pool *pool = wl_shm_create_pool(shm, fd, int(size));
    ::wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);
    return buffer;
}

} // namespace

struct FadeOverlay::Impl {
    ::wl_surface *surface = nullptr;
    ::wl_subsurface *subsurface = nullptr;
    ::wl_buffer *buffer = nullptr;
    ::wl_callback *frameCallback = nullptr;
    QtWayland::wp_viewport viewport;
    QtWayland::wp_alpha_modifier_surface_v1 alpha;
    std::function<void()> onPresented;

    static const wl_callback_listener FrameListener;

    static void handleFrameDone(void *data, ::wl_callback *cb, uint32_t)
    {
        auto *self = static_cast<Impl *>(data);
        wl_callback_destroy(cb);
        if (self->frameCallback == cb)
            self->frameCallback = nullptr;
        if (auto fn = std::move(self->onPresented))
            fn();
    }

    ~Impl()
    {
        if (frameCallback)
            wl_callback_destroy(frameCallback);
        // Spec requires destroying alpha_modifier_surface and viewport before wl_surface.
        if (alpha.isInitialized())
            alpha.destroy();
        if (viewport.isInitialized())
            viewport.destroy();
        if (subsurface)
            wl_subsurface_destroy(subsurface);
        if (surface)
            wl_surface_destroy(surface);
        if (buffer)
            wl_buffer_destroy(buffer);
        FadeOverlay::flushDisplay();
    }
};

const wl_callback_listener FadeOverlay::Impl::FrameListener = {
    &FadeOverlay::Impl::handleFrameDone,
};

FadeOverlay::FadeOverlay(std::unique_ptr<Impl> impl)
    : d(std::move(impl))
{
}

FadeOverlay::~FadeOverlay() = default;

void FadeOverlay::initGlobals()
{
    if (!qGuiApp || !qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>())
        return;
    global<AlphaModifier>();
    global<Viewporter>();
}

std::unique_ptr<FadeOverlay> FadeOverlay::create(QWindow *parent, const QImage &image, QSize logicalSize)
{
    if (!parent || image.isNull() || logicalSize.isEmpty())
        return nullptr;

    auto *waylandApp = qGuiApp ? qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>() : nullptr;
    auto *waylandWin = dynamic_cast<QtWaylandClient::QWaylandWindow *>(parent->handle());
    if (!waylandApp || !waylandWin)
        return nullptr;

    ::wl_surface *parentSurface = waylandWin->wlSurface();
    QtWaylandClient::QWaylandDisplay *display = waylandWin->display();
    ::wl_compositor *compositor = waylandApp->compositor();
    ::wl_subcompositor *subcomp = getSubcompositor(display);
    ::wl_shm *shm = (display && display->shm()) ? display->shm()->object() : nullptr;

    AlphaModifier &alphaMod = global<AlphaModifier>();
    Viewporter &viewporter = global<Viewporter>();
    if (!parentSurface || !compositor || !subcomp || !shm || !alphaMod.isActive() || !viewporter.isActive())
        return nullptr;

    ::wl_buffer *buffer = createShmBuffer(shm, image);
    if (!buffer)
        return nullptr;

    auto impl = std::make_unique<Impl>();
    impl->buffer = buffer;
    impl->surface = wl_compositor_create_surface(compositor);
    impl->subsurface = wl_subcompositor_get_subsurface(subcomp, impl->surface, parentSurface);
    wl_subsurface_set_position(impl->subsurface, 0, 0);
    wl_subsurface_place_above(impl->subsurface, parentSurface);

    // Pass pointer events straight through to the wallpaper widget underneath.
    ::wl_region *empty = wl_compositor_create_region(compositor);
    wl_surface_set_input_region(impl->surface, empty);
    wl_region_destroy(empty);

    impl->viewport.init(viewporter.get_viewport(impl->surface));
    impl->viewport.set_destination(logicalSize.width(), logicalSize.height());

    impl->alpha.init(alphaMod.get_surface(impl->surface));
    impl->alpha.set_multiplier(scale32(1.0));

    wl_surface_attach(impl->surface, impl->buffer, 0, 0);
    wl_surface_damage_buffer(impl->surface, 0, 0, image.width(), image.height());
    // Committed while synchronized: atomically appears with the parent's next commit of the new image.
    wl_surface_commit(impl->surface);

    return std::unique_ptr<FadeOverlay>(new FadeOverlay(std::move(impl)));
}

void FadeOverlay::beginFade(std::function<void()> onPresented)
{
    if (!d || !d->subsurface || !d->surface)
        return;
    wl_subsurface_set_desync(d->subsurface);
    if (onPresented) {
        d->onPresented = std::move(onPresented);
        d->frameCallback = wl_surface_frame(d->surface);
        wl_callback_add_listener(d->frameCallback, &Impl::FrameListener, d.get());
        wl_surface_commit(d->surface);
    }
    flushDisplay();
}

void FadeOverlay::setOpacity(qreal opacity)
{
    if (!d || !d->alpha.isInitialized() || !d->surface)
        return;
    d->alpha.set_multiplier(scale32(opacity));
    wl_surface_commit(d->surface);
}

void FadeOverlay::flushDisplay()
{
    if (auto *waylandApp = qGuiApp ? qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>() : nullptr)
        wl_display_flush(waylandApp->display());
}

} // namespace wallpaper
