# Development Notes

## Build deb package

### Install dependancies

1. `sudo apt install dh-make devscripts`

### Generate the debian changelog

1. `./docs/convert-changelog.sh ./docs/changelog.md ./debian/changelog`

### Create package
Note: replace `0.7.8` with the correct version number of the release.

1. Modify `debian/changelog`
2. Copy / rename `forest` to a build directory named `forest-0.7.8`
3. `cd forest-0.7.8`
4. `dh_make -e <email address> -c lgpl3 --createorig`
5. `debuild -us -uc`

## Include deb in repo

`reprepro includedeb forest /path/to/forest_0.7.8-1_amd64.deb`

## Debugging Wayland/Qt protocol issues (window roles, popups, layer-shell)

Static reasoning about Qt/QtWaylandClient/LayerShellQt internals is
unreliable for this class of bug — their source has moved around a lot
between Qt5/Qt6 and between installed-package versions vs. whatever branch
a GitHub search happens to land on. Wire-tracing the actual protocol
traffic settles it far more reliably:

1. Launch Biome nested inside the current X11 session as a fast,
   disposable, ground-truth compositor (see `biome/docs/history.md`'s "Phase
   0 — dev loop" — wlroots auto-detects the nested X11 backend):
   ```sh
   env -u WAYLAND_DISPLAY DISPLAY=:0 WLR_BACKENDS=x11 \
     /path/to/biome/build/core/biome
   ```
   (prints the `WAYLAND_DISPLAY` it picked, e.g. `wayland-0`).
2. Write a minimal throwaway Qt6 widgets program reproducing the exact
   shape under test (window flags, transient-parent calls, timing of
   `useLayerShell()` relative to `QApplication`, a fake non-toplevel
   "panel" frame if that's what's being tested) — link `Qt6::Widgets` and
   `LayerShellQt::Interface`.
3. Run it against the nested instance with protocol tracing on:
   ```sh
   env -u DISPLAY WAYLAND_DISPLAY=wayland-0 QT_QPA_PLATFORM=wayland \
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

(Originally written while diagnosing why some panel popups picked up a
Biome-drawn border — see `panel/panel-library/popup.h`'s constructor and
`positionOnLauncher()` for the resulting fixes and their rationale.)
