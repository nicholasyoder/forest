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
- **`biome/core/fade_config.cpp`** (separate repo, own history — not part of
  this branch or this list's revert tracking) — `fadingNamespaces`/
  `scanoutFadingNamespaces` were read via `QVariant::toString()`, which
  silently returns `""` once a key holds more than one comma-separated
  namespace (QSettings' IniFormat auto-detects the unescaped comma on
  read-back and hands the value back as a `QStringList`, which `toString()`
  doesn't rejoin). A single namespace round-tripped fine as a plain string,
  which is why this went unnoticed until `scanoutFadingNamespaces` needed a
  second entry (`forest-startup`, alongside `forest-logout-dim`) here — fixed
  to read via `toStringList()` instead, which normalizes both shapes.
- **`biome/desktop/layer_shell.cpp`** (same repo/history note as above) —
  `scanoutFadingNamespaces`' background snapshot used to be captured once at
  map time, which is wrong for a surface that maps *before* the real desktop
  has rendered anything (forest-startup's cover has to, to hide plugin
  startup) — it now re-captures fresh right as fade-out begins
  (`capture_background_snapshot()`, called from both `scanout_fade_create()`
  and `scanout_fade_start_fade_out()`), which in turn needed the
  currently-visible overlay's own `scene_buffer` hidden while capturing (same
  as the client's real content already was), or it captured itself as "the
  background". No behavior change for `forest-logout-dim`, whose background
  is already static throughout its lifetime either way.

## Revert or replace before this branch is done (Wayland-testing scaffolding)

- ~~`library/xcbutills/xcbutills.cpp`, `Xcbutills::setPartialStrut()`~~ —
  **resolved, same day.** Workstream A's Forest-side landed:
  `panel/panel-app/geometrymanager.cpp` now drives a real
  `LayerShellQt::Window` (anchors + `setExclusiveZone()`) instead of raw XCB
  struts, `desktop/desktop-app/wallpaperwidget.cpp` is a `LayerBackground`
  layer surface, and `Xcbutills::setPartialStrut()` — guard included — was
  deleted outright from `xcbutills.{h,cpp}` rather than reverted, exactly as
  planned here.

- ~~Three hardcoded `"Forest-wayland"` config-org overrides~~ — **resolved**.
  `library/pluginutills/pluginutills.cpp`, `panel/panel-app/panel.h`, and
  `services/services-app/hotkeys/foresthotkeys.cpp` each temporarily swapped
  their normal `QSettings("Forest", ...)` for `QSettings("Forest-wayland",
  ...)` so Wayland testing read a separate config copy instead of the live
  X11 one. All three have since reverted back to `"Forest"` (confirmed by
  grep — no `"Forest-wayland"` string remains anywhere in the tree).

## Local machine state (not in git — won't show up in `git diff`)

**Note:** the two `~/.config/Forest-wayland/*.conf` entries below predate
the "Forest-wayland" config-org revert above. Now that the code reads
`QSettings("Forest", ...)` again everywhere, these settings need to live in
`~/.config/Forest/{Forest,Panel}.conf` directly instead — worth confirming
on the test machine that `hotkeys/`, `systray`, `windowlist`, and
`deskswitch` are all in the expected enabled/disabled state there, since
the old `-wayland`-suffixed files are no longer read by anything.

- `[hotkeys]` — the real hotkey list binds through
  `org.freedesktop.portal.GlobalShortcuts` now (not `XGrabKey`).
- `Panel.conf` plugin state — `systray` (plug-0008): last known
  `enabled=false`. `windowlist` (plug-0004): re-enabled once it was ported
  to `wlr-foreign-toplevel-management-unstable-v1`. `deskswitch`
  (plug-0009): re-enabled once ported to the `ext-workspace-v1` +
  `org.biome.Workspaces` hybrid.
- **`~/.config/Forest/Biome.conf`** — `[LayerShell]/scanoutFadingNamespaces`
  must include `forest-startup` (alongside the existing `forest-logout-dim`)
  for the main `forest` process's fullscreen startup black-overlay
  (`forest/forest.cpp`, `layeroverlay` scope `"forest-startup"`) to fade out
  smoothly under Biome. It belongs in `scanoutFadingNamespaces`, not
  `fadingNamespaces` — it's fullscreen, same as `forest-logout-dim`, not a
  small partial-screen surface like the `forest-logout` dialog. This key has
  no shipped default/template anywhere in the tree — absent config means an
  empty set means no fade at all (`biome/core/fade_config.h`) — so this is
  purely local/manual state, same as the hotkeys config above. Not yet
  reflected in any packaging — needs a real default before this branch
  ships, not just a personal dev workaround. Requires a Biome build with the
  multi-value `scanoutFadingNamespaces` fix below — an older Biome silently
  drops fading for both namespaces sharing that key once it holds more than
  one value.
