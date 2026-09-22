# Development Notes

## Build deb package

`debian/` is already set up (native source format, no upstream tarball) —
there's no `dh_make` scaffolding step; that's only for a package that has no
`debian/` directory yet.

### Install dependencies

1. `sudo apt build-dep .` — installs everything listed in `debian/control`'s
   `Build-Depends`
2. `sudo apt install devscripts` — only needed if using `debuild` below;
   `dpkg-buildpackage` alone doesn't require it

### Generate the debian changelog

1. `./docs/convert-changelog.sh ./docs/changelog.md ./debian/changelog`

### Build the package

Note: replace `0.8.0` with the correct version number of the release.

1. Modify `debian/changelog`
2. From `forest/`: `dpkg-buildpackage -us -uc -b` (or `debuild -us -uc` to
   also run `lintian` on the result automatically)

This produces `../forest_0.8.0_amd64.deb`, `../forest-greeter_0.8.0_amd64.deb`,
and `-dbgsym` packages for each, in the parent directory. It does not install
anything locally.

It leaves build byproducts in the tree (`obj-*-linux-gnu/`, `debian/forest/`,
`debian/forest-greeter/`, etc. — all gitignored). Remove them with:

```
dpkg-buildpackage -Tclean
```

## Include deb in repo

`reprepro includedeb forest /path/to/forest_0.7.8-1_amd64.deb`

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

(Originally written while diagnosing why some panel popups picked up a
Biome-drawn border — see `panel/panel-library/popup.h`'s constructor and
`positionOnLauncher()` for the resulting fixes and their rationale.)
