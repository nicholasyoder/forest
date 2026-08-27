# Temporary changes for Wayland testing (`wayland` branch)

Written 2026-08-22 while unblocking `forest` from segfaulting under Wayland so
Phase 4 Workstream A (layer-shell for panel/desktop) can be tested. Everything
below is scoped to this `wayland` branch — an X11 build is a separate branch
switch, per the decision not to add runtime platform branching in the code.

## Keep permanently (real bug fixes, harmless on X11 — not branch-specific)

- **`library/xcbutills/xcbutills.cpp`** — `Xcbutills::conn` static initializer
  and `Xcbutills::display()` now null-check
  `qApp->nativeInterface<QNativeInterface::QX11Application>()` before
  dereferencing it, instead of crashing. Under X11 that interface is always
  present, so this never changes X11 behavior — it only stops a null-pointer
  segfault that a missing/failed X11 platform integration would otherwise
  cause unconditionally, on any platform.
- **`library/xcbutills/numlock.cpp`** — same null-guard in
  `numlock::enableNumlock()`.
- **`build/install_to_staging.sh`** — `--prefix` was `/opt/forest-build/`,
  one directory short of where `/usr/bin/forest` and `/usr/lib/forest`
  actually symlink to (`/opt/forest-build/usr/`). This bug predates the
  Wayland work and silently made the script a no-op for months (confirmed via
  file mtimes) — fixed to `/opt/forest-build/usr/`, matching the `usr/share/*`
  copies already in the same script. Not Wayland-specific, keep on every
  branch.

## Revert or replace before this branch is done (Wayland-testing scaffolding)

- ~~`library/xcbutills/xcbutills.cpp`, `Xcbutills::setPartialStrut()`~~ —
  **resolved, same day.** Workstream A's Forest-side landed:
  `panel/panel-app/geometrymanager.cpp` now drives a real
  `LayerShellQt::Window` (anchors + `setExclusiveZone()`) instead of raw XCB
  struts, `desktop/desktop-app/wallpaperwidget.cpp` is a `LayerBackground`
  layer surface, and `Xcbutills::setPartialStrut()` — guard included — was
  deleted outright from `xcbutills.{h,cpp}` rather than reverted, exactly as
  planned here. See `biome/docs/phase4-plan.md` Workstream A's session log
  for the full writeup.

- **Three hardcoded `"Forest-wayland"` config-org overrides** — each swaps
  the app's normal `QSettings("Forest", ...)` for `QSettings("Forest-wayland",
  ...)` so Wayland testing reads a separate copy of the config instead of the
  live X11 one. Flat hardcoded strings by design (no `platformName()` check),
  per the decision to keep platform selection to branch switches, not runtime
  detection. **All three must revert to `"Forest"` before this branch merges
  back**, or the merged build will silently stop reading the real config on
  every platform:
  - `library/pluginutills/pluginutills.cpp:15` —
    `QSettings("Forest-wayland","Forest")` in `get_plugin_paths()`. Controls
    which top-level app plugins (`desktop-app`/`panel-app`/`services-app`)
    load.
  - `panel/panel-app/panel.h:63` — `QSettings("Forest-wayland","Panel")`.
    Controls panel's own sub-plugin list (`windowlist`/`deskswitch`/
    `systray`/etc.) and general panel settings.
  - `services/services-app/hotkeys/foresthotkeys.cpp:49` —
    `QSettings("Forest-wayland","Forest")` in `loadhotkeys()`. Controls the
    configured global-hotkey list.

  Note: `panel/panel-library/popup.h:132` still has the *original*
  `QSettings("Forest","Panel")` — deliberately left untouched since it's
  unrelated to plugin loading (popup positioning), but it's a reminder that
  more `QSettings("Forest", ...)` call sites likely exist elsewhere in the
  tree that never got audited for this split (see chat note from
  2026-08-22 about theming/other plugin settings still reading the X11
  config under Wayland).

## Local machine state (not in git — won't show up in `git diff`)

- **`~/.config/Forest-wayland/Forest.conf`** — `[hotkeys]` section restored
  2026-08-26 (full copy of the real hotkey list from `~/.config/Forest/`)
  now that Workstream C (`biome/docs/phase4-plan.md`) has landed — the
  `hotkeys/` code now binds through `org.freedesktop.portal.GlobalShortcuts`
  instead of `XGrabKey`. Not yet manually confirmed by the user.
- **`~/.config/Forest-wayland/Panel.conf`** — `systray` (plug-0008) still
  set `enabled=false`. `windowlist` (plug-0004) was re-enabled 2026-08-23
  once Workstream B (see `biome/docs/phase4-plan.md`) landed and was
  manually confirmed on both sides — it now runs against
  `wlr-foreign-toplevel-management-unstable-v1` instead of
  `KX11Extras`/`Xcbutills::*`. `deskswitch` (plug-0009) stays
  `enabled=false` — still unported X11-only code, gated on Workstream D
  (workspaces, not started).
