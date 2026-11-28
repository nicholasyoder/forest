// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FADEOVERLAY_H
#define FADEOVERLAY_H

#include <QImage>
#include <QSize>
#include <functional>
#include <memory>

class QWindow;

namespace wallpaper {

// GPU-composited fade overlay using a wl_subsurface + wp_alpha_modifier_v1 + wp_viewporter.
// Holds the old wallpaper above the main surface and ramps its alpha to 0 with zero per-frame buffer uploads.
class FadeOverlay {
public:
    ~FadeOverlay();

    FadeOverlay(const FadeOverlay &) = delete;
    FadeOverlay &operator=(const FadeOverlay &) = delete;

    // Binds the Wayland globals lazily when running on Wayland.
    static void initGlobals();

    // Creates a synchronized subsurface carrying `image` (opaque 1.0) above `parent`.
    // Returns null if not on Wayland or the compositor lacks wp_alpha_modifier_v1 / wp_viewporter.
    static std::unique_ptr<FadeOverlay> create(QWindow *parent, const QImage &image, QSize logicalSize);

    // Switches to desynchronized mode after the parent has committed the new image underneath,
    // and invokes `onPresented` once the compositor has scanned out the initial frame.
    void beginFade(std::function<void()> onPresented);

    // Updates compositor-side surface alpha without re-uploading pixels.
    void setOpacity(qreal opacity);

    static void flushDisplay();

private:
    struct Impl;
    explicit FadeOverlay(std::unique_ptr<Impl> impl);
    std::unique_ptr<Impl> d;
};

} // namespace wallpaper

#endif // FADEOVERLAY_H
