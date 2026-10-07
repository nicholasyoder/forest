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

## Display settings

Three parts, all on `wlr-output-management-unstable-v1` plus Forest's own
D-Bus, so they work on any wlroots compositor (checked on Biome and sway):

- **`library/outputs`**: hand-bound protocol client (`OutputManager`;
  libkscreen has no wlroots backend), the profile model (`DisplayProfiles`)
  and layout fixups (`layoutedit`).
- **`displays` service in `services-app`** (`org.forest`
  `/org/forest/displays`, interface `org.forest.displays`). It is the only
  writer of `~/.config/Forest/Displays.conf`. It auto-picks a profile at
  login and on hotplug, applies layouts and profiles, draws the
  keep-or-revert and identify cards, and writes `display/primary_screen`
  (emitting `primaryChanged`, which `ScreenTracker::primary_changed`
  forwards).
- **System settings → Displays**: an editor. It reads heads directly and
  `test`s edits, but every apply and save goes through the service.

Behaviour worth knowing:

- **Matching.** An output's identity key is `make|model|serial` when the
  serial is non-empty and unique among connected heads, else the connector.
  A profile matches when its key set equals the connected set; the most
  recently used match wins. Identical monitors without serials are keyed by
  connector, so swapping their cables swaps their settings.
- **Apply and Save are separate.** Applying never writes a profile. The
  active profile is whichever saved one equals the live layout (and
  primary), if any.
- **Confirm/revert** covers changes to enabled, mode, scale or transform,
  not position or primary. The revert target is kept in `Displays.conf`
  (`pending_revert`) until Keep/Revert, so a service restart mid-confirm
  still reverts. Biome has already persisted the unconfirmed layout by then.
  A hotplug mid-confirm switches to a matching profile, or else reverts onto
  the heads that are still connected.
- **Biome persists every successful protocol apply** (from any client) to
  `[Outputs]` in `~/.config/Biome/Biome.conf` and restores it at startup.
  Forest never writes `Biome.conf`.
- **Profile hotkeys** are ordinary `[hotkeys]` entries with
  `DBUS:…,method=applyProfile,arg=<profile id>` (or `nextProfile`),
  parsed/formatted by `library/hotkeyconfig`. Renaming a profile updates
  default descriptions, and deleting one removes its hotkeys.

Testing without extra monitors: headless Biome (fake heads, one custom mode
each). Override `XDG_CONFIG_HOME`, or its applies overwrite the real
`Biome.conf`:

```sh
XDG_CONFIG_HOME=/tmp/biome-cfg WLR_BACKENDS=headless WLR_HEADLESS_OUTPUTS=2 \
  WLR_RENDERER=pixman WLR_LIBINPUT_NO_DEVICES=1 biome -s <client>
```

Headless sway takes the same `WLR_*` variables, and `swaymsg create_output`
hotplugs another head. Plugin paths are hardcoded to `/usr/lib/forest`, so
to test a fresh `libservices-app.so` without staging it, load it from a
throwaway `QPluginLoader` program (register `org.forest` first, disable
quit-on-last-window-closed) under `dbus-run-session`, and drive it with
`gdbus call` and `wlr-randr`.

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
