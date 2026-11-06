// SPDX-License-Identifier: LGPL-3.0-or-later

#include "layeroverlay.h"

#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QWaylandClientExtension>
#include <qguiapplication_platform.h>

#include <QtWaylandClient/private/qwaylandscreen_p.h>

#include <wayland-client.h>

#include <algorithm>
#include <cmath>

#include "qwayland-single-pixel-buffer-v1.h"
#include "qwayland-viewporter.h"
#include "qwayland-wlr-layer-shell-unstable-v1.h"

namespace {

class LayerShell : public QWaylandClientExtensionTemplate<LayerShell>, public QtWayland::zwlr_layer_shell_v1{
public:
    LayerShell() : QWaylandClientExtensionTemplate<LayerShell>(1){ initialize(); }
};

class Viewporter : public QWaylandClientExtensionTemplate<Viewporter>, public QtWayland::wp_viewporter{
public:
    Viewporter() : QWaylandClientExtensionTemplate<Viewporter>(1){ initialize(); }
};

class SinglePixelBufferManager : public QWaylandClientExtensionTemplate<SinglePixelBufferManager>,
                                 public QtWayland::wp_single_pixel_buffer_manager_v1{
public:
    SinglePixelBufferManager() : QWaylandClientExtensionTemplate<SinglePixelBufferManager>(1){ initialize(); }
};

// Process-wide: one bind per global, shared by every overlay.
template<class T> T &global(){
    static T *instance = new T;
    return *instance;
}

uint32_t scale32(qreal unit){
    return static_cast<uint32_t>(std::lround(std::clamp(unit, 0.0, 1.0) * 0xFFFFFFFFu));
}

} // namespace

struct layeroverlay::Impl : public QtWayland::zwlr_layer_surface_v1{
    layeroverlay *owner = nullptr;
    QColor color;
    bool passInput = false;
    ::wl_surface *surface = nullptr;
    QtWayland::wp_viewport viewport;

    ~Impl() override{ teardown(); }

    void teardown(){
        if (isInitialized())
            destroy();
        if (viewport.isInitialized())
            viewport.destroy();
        if (surface){
            wl_surface_destroy(surface);
            surface = nullptr;
        }
    }

    void zwlr_layer_surface_v1_configure(uint32_t serial, uint32_t width, uint32_t height) override{
        ack_configure(serial);
        if (!width || !height)
            return;
        viewport.set_destination(width, height);

        // Protocol colors are premultiplied.
        const qreal alpha = color.alphaF();
        ::wl_buffer *buffer = global<SinglePixelBufferManager>().create_u32_rgba_buffer(
            scale32(color.redF() * alpha), scale32(color.greenF() * alpha), scale32(color.blueF() * alpha), scale32(alpha));
        wl_surface_attach(surface, buffer, 0, 0);

        ::wl_compositor *compositor = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>()->compositor();
        if (color.alpha() == 255){
            ::wl_region *opaque = wl_compositor_create_region(compositor);
            wl_region_add(opaque, 0, 0, width, height);
            wl_surface_set_opaque_region(surface, opaque);
            wl_region_destroy(opaque);
        }
        if (passInput){
            ::wl_region *none = wl_compositor_create_region(compositor);
            wl_surface_set_input_region(surface, none);
            wl_region_destroy(none);
        }
        wl_surface_commit(surface);
        wl_buffer_destroy(buffer); // After the commit: destroying a pending buffer is an unspecified null attach.
    }

    void zwlr_layer_surface_v1_closed() override{
        owner->close();
    }
};

layeroverlay::layeroverlay(const QColor &color, Layer layer, const QString &scope, QScreen *screen, bool passInput) : d(new Impl){
    d->owner = this;
    d->color = color;
    d->passInput = passInput;

    auto *wayland_app = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    auto *wayland_screen = dynamic_cast<QtWaylandClient::QWaylandScreen *>(screen->handle());
    LayerShell &shell = global<LayerShell>();
    if (!wayland_app || !wayland_screen || !shell.isActive()
        || !global<Viewporter>().isActive() || !global<SinglePixelBufferManager>().isActive()){
        qWarning() << "layeroverlay: unavailable: app" << bool(wayland_app) << "screen" << bool(wayland_screen)
                   << "layer-shell" << shell.isActive() << "viewporter" << global<Viewporter>().isActive()
                   << "single-pixel" << global<SinglePixelBufferManager>().isActive();
        deleteLater();
        return;
    }

    d->surface = wl_compositor_create_surface(wayland_app->compositor());
    d->viewport.init(global<Viewporter>().get_viewport(d->surface));
    d->init(shell.get_layer_surface(d->surface, wayland_screen->output(), static_cast<uint32_t>(layer), scope));
    using Surface = QtWayland::zwlr_layer_surface_v1;
    d->set_anchor(Surface::anchor_top | Surface::anchor_bottom | Surface::anchor_left | Surface::anchor_right);
    d->set_exclusive_zone(-1);
    d->set_keyboard_interactivity(Surface::keyboard_interactivity_none);
    wl_surface_commit(d->surface); // No buffer yet: the compositor answers with configure.

    connect(screen, &QObject::destroyed, this, &layeroverlay::close);
}

layeroverlay::~layeroverlay() = default;

QList<QPointer<layeroverlay>> layeroverlay::showOnAllScreens(const QColor &color, Layer layer, const QString &scope, bool passInput){
    QList<QPointer<layeroverlay>> overlays;
    for (QScreen *screen : QGuiApplication::screens())
        overlays << new layeroverlay(color, layer, scope, screen, passInput);
    return overlays;
}

void layeroverlay::close(){
    d->teardown();
    // Flush now: callers may quit right after.
    if (auto *wayland_app = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>())
        wl_display_flush(wayland_app->display());
    deleteLater();
}
