# Development Notes

## Release packaging

Building the `.deb`s is covered in the README's Packaging section. For a
release:

1. Add a `* Release X.Y.Z - YYYY-MM-DD` entry to `docs/changelog.md` and bump
   `project(... VERSION ...)` in `CMakeLists.txt`.
2. Regenerate `debian/changelog` from it (don't edit it by hand):
   `./docs/convert-changelog.sh ./docs/changelog.md ./debian/changelog`
3. Build with `debuild -us -uc`, which runs lintian (it flags changelog lines
   over 80 columns).

## Include deb in repo

`reprepro includedeb forest /path/to/forest_0.7.8-1_amd64.deb`

## Recovering from a stuck lock

If `forest-lockscreen` hangs while testing, switch to another VT and run
`swaylock` against Biome's `WAYLAND_DISPLAY`; Biome lets it take over the
lock, then unlock with it.

## Debugging Wayland/Qt protocol issues (window roles, popups, layer-shell)

Static reasoning about Qt/QtWaylandClient/LayerShellQt internals is
unreliable for this class of bug — their source has moved around a lot
between Qt5/Qt6 and between installed-package versions vs. whatever branch
a GitHub search happens to land on. Wire-tracing the actual protocol
traffic settles it far more reliably:

1. Launch Biome nested inside the running Biome session as a fast,
   disposable, ground-truth compositor (see `biome/docs/history.md`'s "Phase
   0 — dev loop" — wlroots auto-detects the nested Wayland backend from the
   parent's `WAYLAND_DISPLAY`):
   ```sh
   WLR_BACKENDS=wayland /path/to/biome/build/core/biome
   ```
   (prints the `WAYLAND_DISPLAY` it picked, e.g. `wayland-1` — not the
   parent's socket).
2. Write a minimal throwaway Qt6 widgets program reproducing the exact
   shape under test (window flags, transient-parent calls, timing of
   `useLayerShell()` relative to `QApplication`, a fake non-toplevel
   "panel" frame if that's what's being tested) — link `Qt6::Widgets` and
   `LayerShellQt::Interface`.
3. Run it against the nested instance with protocol tracing on:
   ```sh
   env -u DISPLAY WAYLAND_DISPLAY=wayland-1 QT_QPA_PLATFORM=wayland \
     WAYLAND_DEBUG=1 ./throwaway-binary
   ```
4. Grep the trace for `get_toplevel` vs `get_popup` vs `get_layer_surface` /
   `set_anchor` / `set_size` — this settles "did it become a
   toplevel/popup/layer-surface" and "what size/anchors did it actually
   negotiate". Always check the installed `dpkg -l` version of whatever Qt
   library is in question and fetch the matching source tag, not
   `dev`/`master`, when cross-referencing behavior against source.

No real mouse/keyboard interaction is needed for reproducing a
hotkey/D-Bus-triggered code path — a `QTimer::singleShot` calling `show()`
directly reproduces it fine.
