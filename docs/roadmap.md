# Forest Roadmap

Forward-looking, Forest-specific work items, split by size. Several features
are blocked on protocol work that has to land in Biome first (see
`biome/docs/roadmap.md`). Kept at bullet-list altitude deliberately: when a
feature is actually picked up, draft a real implementation plan for it then
(a new `docs/<item>-plan.md`, following the existing `docs/greeter-plan.md` /
`docs/qmenu-migration-plan.md` pattern), rather than designing it here ahead
of time. Small fixes don't need a plan doc.

## Small fixes

Bugs and cleanups — roughly a single sitting each, no design work needed.

### Bugs

- **Desktop-icons surface needs `KeyboardInteractivityOnDemand`.**
  `wallpaperwidget` requests `None`; icon rename, Delete/Shift+Delete and
  Ctrl-click multi-select only work because Biome grants focus to any clicked
  surface regardless. Switch the primary-screen (icons) surface to
  `OnDemand` *before* Biome fixes that (Biome roadmap, Known issues), or
  desktop keyboard input breaks.
- **`DesktopNames` mismatch.** `wayland-sessions/Forest.desktop` has
  `DesktopNames=Forest`, but `startforest-wayland` exports
  `XDG_CURRENT_DESKTOP=Forest:biome` (needed for `biome-portals.conf`).
  Make them agree (likely `DesktopNames=Forest;biome`).
- **Battery monitor doesn't refresh on panel plugin reload** (#38).

### Desktop

- **Desktop icon multi-drag.** Rubber-band multi-select works, but dragging
  only moves the grabbed icon (`desktopicon` / `iconswidget::handleicondragged`).
  Move the whole selection, keeping relative positions.

### Hotkeys

- **Hotkeys stay paused if settings dies mid-capture.** `pauseHotkeys` has no
  owner; watch the caller's bus name (`QDBusServiceWatcher`) and resume when
  it vanishes.
- **Hotkeys don't recover from portal failures.** A failed `createSession`
  is terminal (no retry), and a portal restart or `Session::Closed` leaves
  `sessionOpen` true with hotkeys dead until relog. Watch the
  `org.freedesktop.portal.Desktop` owner and `Session::Closed`, and retry.
- **Hotkey capture can't record a bare Meta tap.** `edithotkeywidget::keyPressEvent`
  appends `Meta+` and waits for a non-modifier key; with no `keyReleaseEvent`
  it never produces the `Meta` value `foresthotkeys` understands. Pre-existing.
- **Show desktop hotkey.** The X11 `showdesktop` slot and its Meta+D default
  were dropped. No standard protocol exists; the decoupled route is
  `set_minimized` on every wlr-foreign-toplevel handle, so it belongs next to
  windowlist's handles in the panel. Existing users' `Forest.conf` still has
  the dead `item-0003` entry — rewrite it via `settings_upgrade_manager.cpp`.

### Cleanups

- **Rename the panel `seperator` config value to `separator`.** The display
  string is fixed, but `plug-NNNN/path=seperator` is still what
  `Panel.conf` stores (`panel.cpp`, `panelsettings.cpp`,
  `etc/forest/Panel.conf`). Rename all three together and add an
  `upgrade_x_y_z()` to `forest/settings_upgrade_manager.cpp` that rewrites
  existing users' `~/.config/Forest/Panel.conf`.

## Features

Larger work — write a plan doc when picked up.

### Ready

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
- **Ship a Biome config with the Forest package.** Smaller than the others
  but needs a cross-repo design decision. Forest's layer-shell surfaces need
  `[LayerShell]/scanoutFadingNamespaces=forest-logout-dim,forest-startup`
  (and `fadingNamespaces` for `forest-logout`) in Biome's config to fade at
  all; neither repo ships a default today, so a fresh install gets no fades.
  Since these are Forest app namespaces, the Forest package should install
  the config rather than Biome hardcoding them. Undecided how: a system-wide
  `/etc` file Biome reads, a drop-in directory, or a compiled-in default.
  Needs Biome's config lookup order checked first (`biome/core/fade_config.cpp`).
- **Network manager plugin.** `nmcontrol` is a stub (static icon, empty
  popup). Needs connection status, Wi-Fi list/connect, wired/VPN toggles.
  Pick a backend first (NetworkManagerQt vs. raw `org.freedesktop.NetworkManager`
  D-Bus).
- **Devices / mount manager panel plugin** (#37). Panel icon to
  mount/unmount/eject removable drives, plus remounting chosen drives at
  login. UDisks2 directly or via Solid; decide together with the battery
  monitor's backend (#38) so both use the same one.
- **Notification position setting.** Popups are anchored bottom-right
  (`notifypopup.cpp`); add a corner choice to the Notifications settings page.
- **Battery monitor: configurable low-battery notifications** (#38).
  Thresholds and actions (warn / suspend / hibernate) in settings.
- **System monitor popups** (#28). Popups for CPU/memory monitors with
  per-core and RAM/swap graphs, and per-sensor graphs for `sensors`.
- **Panel tooltips.** Themed tooltips on panel items (e.g. app name on
  quicklaunch hover). Needs QSS styling to match the theme and custom
  positioning so they open away from the panel edge, not over it.
- **Multiple panels** (#53). E.g. dock at the bottom + status bar at the
  top. `Panel.conf` and panel settings assume a single panel today.
- **Hot corners.** Small layer-shell surfaces in the screen corners (a
  client can't see the global pointer under Wayland), each with a
  user-configurable action (show desktop, open menu, run command, task view
  once it exists).
- **Desktop icons settings page.** Desktop → Icons is a placeholder button.
  Icon size, grid spacing, sort/arrange, which default icons show, etc.
- **Wallpaper slideshow mode** (#21). Cycle through a directory at a
  configurable interval; also add a solid-colour background mode.
- **Desktop widgets** (#54). Windows 7-style gadgets (analog clock, resource
  monitor, weather) on a layer-shell surface, with a panel button to
  show/hide them.
- **Icon theme setting.** Themes → Icon is an empty category. Needs a
  picker plus a decision on how the choice reaches other toolkits (Qt platform
  theme, GTK via gsettings/`settings.ini`), not just Forest's own processes.
- **Unify settings app theme with the desktop theme** (#56). Only
  `base/settings.css` exists, so the settings app ignores the selected theme.
  Add per-variant `settings.css` overrides and reload on theme change.
- **Settings search** (#25). Search individual settings and pages.
- **More hardware info on the About page** (#61). Machine vendor/model,
  GPU, root-disk capacity, BIOS/firmware, boot mode, Secure Boot state,
  motherboard. Maybe a separate "more details" page (lshw-gtk-like).

### Blocked on Biome

- **Session locker** (#5). Blocked on Biome roadmap Phase 6: `ext-idle-notify-v1`
  for idle-triggered lock timing and `wlr-output-power-management-unstable-v1`
  for display blanking (both not yet built); `ext-session-lock-v1` itself is
  already implemented on Biome's side and confirmed working with swaylock.
  A prior plan doc existed but was X11-only and was removed. Write a fresh
  plan against the Wayland protocols above; the PAM/logind side of the old
  plan (PAM auth in a `QThread`, `org.freedesktop.login1`
  `Lock`/`Unlock`/`PrepareForSleep` integration) can likely carry over as-is.
- **Screenshot tool.** Forest currently just depends on `gnome-screenshot`
  (`debian/control`). Under Wayland that needs either portal-based
  screenshot support (`xdg-desktop-portal`'s Screenshot interface, which
  needs Biome as its backend) or a native Forest tool against
  `wlr-screencopy-unstable-v1` / `ext-image-copy-capture-v1`, once Biome
  roadmap Phase 6 lands one of those protocols. Decide native vs.
  portal-based when this is picked up.
- **Clipboard manager.** Clipboard history needs a data-control protocol,
  which Biome doesn't implement yet (Biome roadmap Phase 6).
- **Task view.** Overview of open windows (and workspaces) with live
  thumbnails. Needs per-toplevel capture (`ext-image-copy-capture-v1` with a
  foreign-toplevel capture source), Biome roadmap Phase 6, which needs a
  wlroots bump past 0.18. Windowlist hover previews would come back with it.

### Needs research

- **Panel popups rely on a Biome-only focus exception.** `popup.h` popups
  (main-menu launcher, volume, calendar, battery, sensors, nmcontrol,
  windowlist thumbnails) are non-grabbing `Qt::ToolTip` xdg_popups. Their
  keyboard input (e.g. the main-menu search box) and click-outside-closes
  (`WindowDeactivate`) only work because Biome focuses any layer-shell-owned
  popup (`popup_wants_keyboard_focus()`). On other compositors they get no
  keys and don't close. Likely direction: click-opened popups become grabbing
  `Qt::Popup`s (fresh press serial, like QMenus). The hotkey-opened launcher
  has no serial, so it needs something else, e.g. its own layer surface with
  keyboard interactivity when hotkey-opened. Then Biome drops the exception
  (Biome roadmap, Known issues). Independent of the QMenu migration: QMenus
  grab and don't use it.
- **Combined taskbar and quicklaunch** (#55). Pinned launchers that become
  window buttons when running (KDE icons-only task manager / Plank style).
  The hard part is matching toplevel `app_id`s to `.desktop` files reliably;
  windowlist's `iconresolver` is a starting point.
- **Session save/restore.** Relaunch the apps open at logout. No standard
  Wayland protocol for this yet (session-management is still experimental),
  so the fallback is relaunching `.desktop` entries matched from
  foreign-toplevel `app_id`s, without window positions or app state.

### Low priority

- **Directory menu.** Panel plugin that browses a folder as cascading menus.
  Needs real submenus, so it waits on `docs/qmenu-migration-plan.md`.
- **Theme editor.** Customize accent/highlight colours on top of a theme,
  up to a full theme editor.

### Longer-term (decoupling)

- **Workspaces beyond Biome's model.** deskswitch/windowlist assume one
  ext-workspace group (a single active workspace); a compositor with
  per-output groups would need per-group handling. The window -> workspace
  mapping and "move to desktop" still go through the Biome-only
  `org.biome.Workspaces` (`BiomeWorkspaces`, hidden when absent) — replace it
  with a standard protocol if one appears (ext-workspace has no toplevel
  membership).
