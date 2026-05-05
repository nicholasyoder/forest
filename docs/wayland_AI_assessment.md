# Wayland Migration Notes

## The Core Challenge: Compositor vs. Shell

On X11, Forest is a **desktop shell** that delegates window management (borders, decorations, stacking) to an external WM like xfwm4. On **Wayland, painting window borders requires being the compositor** — there's no separation between the WM and the display server. This fundamentally changes the scope depending on what you want.

---

## Two Paths

### Path A: Remain a Shell (pair with a Wayland compositor)
Replace xfwm4 with a Wayland compositor (KWin, Mutter, or a wlroots-based one). Forest becomes a Wayland client that acts as a shell, but **you don't control window borders** — the compositor does. You could influence borders via compositor themes/plugins but not paint them directly from Forest.

### Path B: Build a Wayland Compositor (full control over borders)
Forest itself becomes the compositor. This gives you complete control over window decorations via **Server-Side Decorations (SSD)**, but it is a significantly larger undertaking.

---

## Specific Changes Required

### 1. Replace `xcbutills` — the biggest chunk
The entire `library/xcbutills/` abstraction layer (window list, raise/minimize/close, EWMH atoms, desktop switching) needs to be rewritten. Wayland equivalents:
- **KWayland** (if staying KDE-adjacent) — has `PlasmaWindow` management protocol
- **wlr-foreign-toplevel-management** — wlroots protocol for listing/managing windows from a shell
- **ext-foreign-toplevel-list** — newer standardized protocol

### 2. Panel → Layer Shell
The panel currently uses `_NET_WM_STRUT` and `Qt::WA_X11NetWmWindowTypeDock`. Replace with **`wlr-layer-shell`** (`zwlr_layer_shell_v1`). Qt has a plugin (`qt-wayland-compositor` or `layer-shell-qt`) that wraps this. This is the most tractable piece.

### 3. System Tray
XEmbed (used in the panel tray) **does not work on Wayland**. Replace with **StatusNotifierItem (SNI)** over D-Bus — most modern apps already support this.

### 4. Screenshots
XCB screenshot code needs to be replaced with **PipeWire + xdg-desktop-portal** (`org.freedesktop.portal.ScreenCast`).

### 5. `forestxcbeventfilter.h` — X11 event loop
All raw XCB event handling needs to be replaced. On Wayland, events come via `wl_display` dispatch or Qt's Wayland platform plugin abstracts this.

### 6. KWindowSystem
KF6's `KWindowSystem` auto-detects X11 vs. Wayland and provides a compatibility layer for many operations (window activation, virtual desktops). This is the **lowest-effort migration path** for high-level WM operations — much of the `xcbutills` usage could be replaced by KWindowSystem calls that work on both platforms.

---

## Effort Summary

| Piece | Effort | Notes |
|---|---|---|
| Panel → layer-shell | Medium | `layer-shell-qt` exists, mostly mechanical |
| System tray → SNI | Low-Medium | D-Bus protocol, well documented |
| xcbutills → KWindowSystem/foreign-toplevel | Medium-High | Large API surface to remap |
| Screenshots → PipeWire portal | Low | Portal API is straightforward |
| **Painting window borders (SSD)** | **Very High** | Requires building a compositor with wlroots or Smithay |
| X11 event filter removal | Medium | Audit all `xcb_` calls |

---

## Recommendation

If the goal is **custom window borders specifically**, that requires writing a compositor (Path B using **wlroots** in C or **Smithay** in Rust). That's roughly the complexity of writing sway or Wayfire from scratch alongside Forest — a multi-month project on its own.

A more pragmatic middle ground: pair Forest with **KWin** (which supports scripted decoration themes) or **Wayfire** (plugin-based, highly customizable borders), and migrate Forest's shell components (panel, tray, desktop) to Wayland protocols incrementally. This gets Wayland and styled borders without writing a compositor.

---

## Key Libraries / Protocols Reference

| Component | X11 | Wayland Replacement |
|---|---|---|
| Window management API | xcb, Xlib | wlr-foreign-toplevel, KWayland |
| Panel anchoring | `_NET_WM_STRUT`, dock type hint | `zwlr_layer_shell_v1` (layer-shell-qt) |
| System tray | XEmbed | StatusNotifierItem (SNI) via D-Bus |
| Screenshots | XCB image capture | PipeWire + xdg-desktop-portal |
| High-level WM ops | KWindowSystem (X11 mode) | KWindowSystem (Wayland mode) |
| Window decorations | External WM (xfwm4) | Compositor SSD or per-app CSD |
| Event loop | XCB event filter | wl_display dispatch / Qt Wayland plugin |
