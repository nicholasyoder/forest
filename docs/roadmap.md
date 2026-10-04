# Forest Roadmap

Forward-looking, Forest-specific work, grouped by target release. Release
assignments are a plan, not a commitment: move items between releases freely
as priorities change. Several items are blocked on protocol work that has to
land in Biome first (see `biome/docs/roadmap.md`, which uses the same
release-grouped layout; Biome releases usually ship alongside Forest's).

Kept at bullet-list altitude deliberately: when a feature is actually picked
up, draft a real implementation plan for it then (a new `docs/<item>-plan.md`,
following the existing `docs/greeter-plan.md` pattern), rather than designing
it here ahead of time. Small fixes don't need a plan doc.

## 0.9.0 — first Wayland release

Ships with Biome 0.1.0. Everything here is required before tagging.

### Blockers

- **Desktop file operations are broken.** Copy/cut/paste/delete and "New
  file" on the desktop do nothing (confirmed): `fmutils.h` and `desktop.cpp`
  call `QProcess::start("rm -r \"...\"")`, and since Qt6 a single string is
  the program name, not a command line. Fix by replacing `fmutils` with
  `library/fileops`, pure Qt: `QFile::moveToTrash` (per-drive trash, replaces
  the hand-rolled home-only trash), threaded copy/move/delete jobs with
  progress, cancel, conflict prompts (overwrite/skip/rename/apply-to-all),
  cross-device moves, and an error report; cut/paste interop via
  `x-special/gnome-copied-files`. Keep the API backend-neutral so a GIO
  backend could slot in if a file manager ever happens (GLib is already
  linked via QtCore; libfm-qt/KIO rejected).
- **Screenshots.** The default Meta+S / Ctrl+Meta+S hotkeys and
  `debian/control` use `gnome-screenshot`, which can't capture under Biome.
  Biome 0.1.0 adds `wlr-screencopy-unstable-v1`; on top of it either depend on
  `grim` (+ `slurp` for regions) or write a native Forest screenshot client.
  Decide when picked up. Active-window capture has no standard path on
  wlroots 0.18 (per-toplevel capture needs 0.19), so that hotkey may become
  region capture instead.
- **Settings upgrade for 0.9.0.** `version` is bumped to `0.9.0` and
  `upgrade_0_9_0()` (`forest/settings_upgrade_manager.cpp`) adds the session
  locker's Meta+L hotkey and settings plugin. Still to add: drop the dead
  show-desktop `item-0003` hotkey, rewrite the `gnome-screenshot` hotkeys to
  whatever replaces them, and rename `plug-NNNN/path=seperator` to
  `separator` in `~/.config/Forest/Panel.conf` (see Cleanups). Keep each step
  idempotent: testers re-run it by resetting `version`.
- **Ship a Biome config with the Forest package.** Forest's layer-shell
  surfaces need `[LayerShell]/scanoutFadingNamespaces=forest-logout-dim,forest-locker-dim,forest-startup`
  (and `fadingNamespaces` for `forest-logout`) in Biome's config to fade at
  all; neither repo ships a default today, so a fresh install gets no fades.
  Since these are Forest app namespaces, the Forest package should install
  the config rather than Biome hardcoding them. Undecided how: a system-wide
  `/etc` file Biome reads, a drop-in directory, or a compiled-in default.
  Needs Biome's config lookup order checked first (`biome/core/fade_config.cpp`).

### Bugs

- **`DesktopNames` mismatch.** `wayland-sessions/Forest.desktop` has
  `DesktopNames=Forest`, but `startforest-wayland` exports
  `XDG_CURRENT_DESKTOP=Forest:biome` (needed for `biome-portals.conf`).
  Make them agree (likely `DesktopNames=Forest;biome`).
- **Hotkeys don't recover from portal failures.** A failed `createSession`
  is terminal (no retry), and a portal restart or `Session::Closed` leaves
  `sessionOpen` true with hotkeys dead until relog. Watch the
  `org.freedesktop.portal.Desktop` owner and `Session::Closed`, and retry.
- **Hotkeys stay paused if settings dies mid-capture.** `pauseHotkeys` has no
  owner; watch the caller's bus name (`QDBusServiceWatcher`) and resume when
  it vanishes.
- **Show desktop hotkey** (regression from 0.8.0). The X11 `showdesktop` slot
  and its Meta+D default were dropped. No standard protocol exists; the
  decoupled route is `set_minimized` on every wlr-foreign-toplevel handle, so
  it belongs next to windowlist's handles in the panel. Re-add the default in
  `etc/forest/Forest.conf` (the 0.9.0 upgrade removes the dead old entry).
- **Hotkey capture can't record a bare Meta tap.** `edithotkeywidget::keyPressEvent`
  appends `Meta+` and waits for a non-modifier key; with no `keyReleaseEvent`
  it never produces the `Meta` value `foresthotkeys` understands. Pre-existing.
- **Battery monitor doesn't refresh on panel plugin reload** (#38).

### Cleanups

- **Rename the panel `seperator` config value to `separator`.** The display
  string is fixed, but `plug-NNNN/path=seperator` is still what
  `Panel.conf` stores (`panel.cpp`, `panelsettings.cpp`,
  `etc/forest/Panel.conf`). Rename all three together; the existing-user
  rewrite goes in `upgrade_0_9_0()` above.

### Release checklist

- `CMakeLists.txt` `project(... VERSION 1.0.0)` → `0.9.0`.
- `docs/changelog.md` and `debian/changelog` entries covering everything
  since v0.8.0 (Wayland/Biome cutover, QMenu migration, autohide and logout
  fixes, plus the items above).
- Fresh-install test in a VM: `biome` + `forest` + `forest-greeter` `.deb`s,
  log in through greetd, check fades, hotkeys, screenshots, lock/blank.
- Merge `develop` → `master`, tag `v0.9.0`, push.
- Delete the merged `wayland` and `qmenu-styling` branches (local and origin).

## 0.10.0 — displays & desktop

- **Display settings plugin.** New `system-settings` plugin for multi-monitor
  configuration (mode/scale/position/rotation). Biome implements
  `wlr-output-management-unstable-v1` (`wlr-randr` works today; Biome rejects
  layouts with gaps between outputs). Biome's own roadmap notes reusing
  `libkscreen`'s existing backend for that protocol rather than hand-binding
  it — worth checking whether Forest should bind `libkscreen` directly too,
  or go through a different Qt-native path, when this is actually designed.
  Ideas to fold in:
  - **Primary screen setting.** `ScreenTracker::primary()` (miscutills) already
    reads `display/primary_screen` (output name) from `Forest.conf`, falling
    back to the top-left screen; panel, desktop icons and logout use it. Only
    the UI is missing. `notifypopup.cpp` should use it when ported (review 6.2).
  - **Display profiles.** Named layouts (which outputs are on, mode/scale/
    position, and the primary screen) stored in Forest's settings and applied
    through the output-management protocol, replacing hand-rolled
    `wlr-randr` scripts and the swap-`Biome.conf`-and-relog workflow.
- **Desktop icons settings page.** Desktop → Icons is a placeholder button.
  Icon size, grid spacing, sort/arrange, which default icons show, etc.
- **Desktop icon multi-drag.** Rubber-band multi-select works, but dragging
  only moves the grabbed icon (`desktopicon` / `iconswidget::handleicondragged`).
  Move the whole selection, keeping relative positions.
- **Wallpaper slideshow mode** (#21). Cycle through a directory at a
  configurable interval; also add a solid-colour background mode.
- **Notification position setting.** Popups are anchored bottom-right
  (`notifypopup.cpp`); add a corner choice to the Notifications settings page.
- **Icon theme setting.** Themes → Icon is an empty category. Needs a
  picker plus a decision on how the choice reaches other toolkits (Qt platform
  theme, GTK via gsettings/`settings.ini`), not just Forest's own processes.
- **Unify settings app theme with the desktop theme** (#56). Only
  `base/settings.css` exists, so the settings app ignores the selected theme.
  Add per-variant `settings.css` overrides and reload on theme change.

### Compositor portability

Forest-side halves of Biome-only focus exceptions. Each must land *before*
Biome removes its exception (Biome roadmap 0.2.0), or Forest input breaks.

- **Desktop-icons surface needs `KeyboardInteractivityOnDemand`.**
  `wallpaperwidget` requests `None`; icon rename, Delete/Shift+Delete and
  Ctrl-click multi-select only work because Biome grants focus to any clicked
  surface regardless. Switch the primary-screen (icons) surface to
  `OnDemand`.
- **Panel popups rely on a Biome-only focus exception** (needs research).
  `popup.h` popups (main-menu launcher, volume, calendar, battery, sensors,
  nmcontrol, windowlist thumbnails) are non-grabbing `Qt::ToolTip` xdg_popups.
  Their keyboard input (e.g. the main-menu search box) and click-outside-closes
  (`WindowDeactivate`) only work because Biome focuses any layer-shell-owned
  popup (`popup_wants_keyboard_focus()`). On other compositors they get no
  keys and don't close. Likely direction: click-opened popups become grabbing
  `Qt::Popup`s (fresh press serial, like QMenus). The hotkey-opened launcher
  has no serial, so it needs something else, e.g. its own layer surface with
  keyboard interactivity when hotkey-opened. Panel QMenus grab and don't use
  the exception, with one exception: mainmenu's app right-click menu is a
  grabbing xdg_popup whose parent is the non-grabbing launcher popup.
  xdg-shell says that's an `invalid_grab` error; wlroots doesn't check, but
  stricter compositors may. A grabbing launcher popup fixes this too.

## 0.11.0 — hardware & system

- **Network manager plugin.** `nmcontrol` is a stub (static icon, empty
  popup; not on the default panel). Needs connection status, Wi-Fi
  list/connect, wired/VPN toggles. Pick a backend first (NetworkManagerQt vs.
  raw `org.freedesktop.NetworkManager` D-Bus).
- **Devices / mount manager panel plugin** (#37). Panel icon to
  mount/unmount/eject removable drives, plus remounting chosen drives at
  login. UDisks2 directly or via Solid; decide together with the battery
  monitor's backend (#38) so both use the same one.
- **Battery monitor: configurable low-battery notifications** (#38).
  Thresholds and actions (warn / suspend / hibernate) in settings.
- **System monitor popups** (#28). Popups for CPU/memory monitors with
  per-core and RAM/swap graphs, and per-sensor graphs for `sensors`.
- **More hardware info on the About page** (#61). Machine vendor/model,
  GPU, root-disk capacity, BIOS/firmware, boot mode, Secure Boot state,
  motherboard. Maybe a separate "more details" page (lshw-gtk-like).
- **Settings search** (#25). Search individual settings and pages.
- **Clipboard manager.** Clipboard history; needs Biome's data-control
  support (Biome roadmap 0.2.0).

## 0.12.0 — panel & windows

- **Panel tooltips.** Themed tooltips on panel items (e.g. app name on
  quicklaunch hover). Needs QSS styling to match the theme and custom
  positioning so they open away from the panel edge, not over it.
- **Multiple panels** (#53). E.g. dock at the bottom + status bar at the
  top. `Panel.conf` and panel settings assume a single panel today.
- **Combined taskbar and quicklaunch** (#55, needs research). Pinned
  launchers that become window buttons when running (KDE icons-only task
  manager / Plank style). The hard part is matching toplevel `app_id`s to
  `.desktop` files reliably; windowlist's `iconresolver` is a starting point.
- **Hot corners.** Small layer-shell surfaces in the screen corners (a
  client can't see the global pointer under Wayland), each with a
  user-configurable action (show desktop, open menu, run command, task view
  once it exists).
- **Directory menu.** Panel plugin that browses a folder as cascading menus.
  Unblocked: panel menus are QMenus with real submenus.

## Later (unscheduled)

- **Task view.** Overview of open windows (and workspaces) with live
  thumbnails. Needs per-toplevel capture (`ext-image-copy-capture-v1` with a
  foreign-toplevel capture source), which needs Biome's wlroots bump past
  0.18 (Biome roadmap, Later). Windowlist hover previews would come back with it.
- **Desktop widgets** (#54). Windows 7-style gadgets (analog clock, resource
  monitor, weather) on a layer-shell surface, with a panel button to
  show/hide them.
- **Session save/restore** (needs research). Relaunch the apps open at
  logout. No standard Wayland protocol for this yet (session-management is
  still experimental), so the fallback is relaunching `.desktop` entries
  matched from foreign-toplevel `app_id`s, without window positions or app state.
- **Theme editor.** Customize accent/highlight colours on top of a theme,
  up to a full theme editor.
- **Workspaces beyond Biome's model** (decoupling). deskswitch/windowlist
  assume one ext-workspace group (a single active workspace); a compositor
  with per-output groups would need per-group handling. The window ->
  workspace mapping and "move to desktop" still go through the Biome-only
  `org.biome.Workspaces` (`BiomeWorkspaces`, hidden when absent) — replace it
  with a standard protocol if one appears (ext-workspace has no toplevel
  membership).
