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
- **Custom-painted panel plugins at fractional scale.** `graphwidget`
  (cpu/memory monitors), `deskbutton`, `battery` and `sensorwidget` paint
  in integer logical pixels. At 1.5x each logical pixel is 1.5 device
  pixels, so 1px lines/columns come out 1 or 2px and bar widths/gaps vary,
  depending on the widget's position. Fix: shared panel-library helper
  that sets the painter to device-pixel units with the origin snapped to a
  device pixel, then lay out each painter in device pixels (constants
  × dpr, rounded). Graph history also has to be per device column.
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
- **Logout dialog relies on Biome's map-time focus.** `logout.cpp` requests
  `OnDemand`; its arrow/Enter/Escape handling works only because Biome
  focuses any non-`None` layer surface on map. Switch to `Exclusive` (a
  modal overlay is the intended use).
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
- **Native screenshot client.** Replaces the grim + slurp script under the
  same `forest-screenshot` name, held until Biome's wlroots bump so it ships
  with window capture rather than as a partial version. Screencopy for
  full/region (region picker on a frozen capture via `layeroverlay`); window
  capture via `ext-image-copy-capture-v1` +
  `ext-foreign-toplevel-image-capture-source-v1`, finding the active window
  with `ToplevelTracker`'s wlr↔ext handle pairing (`library/toplevels`). On
  Wayland the clipboard empties when its owner exits, so copy via a forked
  owner as `wl-copy` does.
- **Desktop widgets** (#54). Windows 7-style gadgets (analog clock, resource
  monitor, weather) on a layer-shell surface, with a panel button to
  show/hide them.
- **Session save/restore** (needs research). Relaunch the apps open at
  logout. No standard Wayland protocol for this yet (session-management is
  still experimental), so the fallback is relaunching `.desktop` entries
  matched from foreign-toplevel `app_id`s, without window positions or app state.
- **Greeter on Biome, shared with the lockscreen.** Move the greeter from
  cage to Biome with one window per output on `loginui`'s per-screen
  background; split `PasswordView` into a card shared with
  `forest-lockscreen`'s `PasswordCard`; maybe rename `greeter.css` (and the
  `greeter_*` object names) to a shared login component. The lockscreen's
  card already follows keyboard focus between screens; do the same in the
  greeter. Also give the greeter and lockscreen their own wallpaper setting
  instead of reusing the desktop wallpaper (`loginui::Wallpaper` reads
  `desktop/wallpaper`).
- **Hotkeys on the lock screen.** Biome hotkeys are off while locked, so
  volume/brightness keys do nothing there. Add a per-hotkey
  `allow_on_lockscreen` flag (visible in hotkey settings, default on for the
  shipped volume/brightness entries) and have `forest-lockscreen` run only
  flagged hotkeys. Opt-in, since a remapped key could otherwise run arbitrary
  commands unauthenticated. Share the action parsing with `foresthotkeys`.
- **Theme editor.** Customize accent/highlight colours on top of a theme,
  up to a full theme editor.
- **Workspaces beyond Biome's model** (decoupling). deskswitch/windowlist
  assume one ext-workspace group (a single active workspace); a compositor
  with per-output groups would need per-group handling. The window ->
  workspace mapping and "move to desktop" still go through the Biome-only
  `org.biome.Workspaces` (`BiomeWorkspaces`, hidden when absent) — replace it
  with a standard protocol if one appears (ext-workspace has no toplevel
  membership).
