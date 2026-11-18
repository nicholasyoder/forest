# Forest Roadmap

Forward-looking, Forest-specific work, grouped by target release. Releases are
meant to be small and frequent: one or two major items plus a few easy ones
each, ordered by how urgently they're needed. Release assignments are a plan,
not a commitment: move items between releases freely as priorities change.
Several items are blocked on protocol work that has to land in Biome first
(see `biome/docs/roadmap.md`, which uses the same release-grouped layout;
Biome releases usually ship alongside Forest's, noted per heading below).

Kept at bullet-list altitude deliberately: when a feature is actually picked
up, draft a real implementation plan for it then (a new `docs/<item>-plan.md`),
rather than designing it here ahead of time. Small fixes don't need a plan doc.

## 0.11.2 — applet, settings & login fixes

- **Monitor, tray, quicklaunch and volume fixes** ([detail](review-2026-11.md#applet-fixes)).
  Incl. no battery warning while charging, working mute toggle.
- **Settings and displays fixes** ([detail](review-2026-11.md#settings-fixes)).
- **Greeter, logout and session fixes** ([detail](review-2026-11.md#login-fixes)).
- **Screenshot fixes, ext-data-control clipboard** ([detail](review-2026-11.md#screenshot-fixes)).
- **Dead-code sweep** ([detail](review-2026-11.md#dead-code)). Removes the `nmcontrol` stub.

## 0.12.0 — desktop & notification polish

Pairs with Biome 0.4.0.

- **Desktop icon view rework** ([detail](review-2026-11.md#desktop-icon-view)). Incremental
  updates, cached label shadows, no ghost icons, grid sized to the usable
  area. Do before the settings page.
- **Desktop icons settings page.** No page for desktop icons yet (Desktop →
  Icons is a placeholder). Icon size, grid spacing, sort/arrange, which
  default icons show, etc.
- **Wallpaper slideshow mode** (#21). Cycle through a directory at a
  configurable interval; also add a solid-colour background mode.
- **Icon theme setting.** No icon theme page yet (Themes → Icon is an empty
  placeholder). Needs a picker plus a decision on how the choice reaches
  other toolkits (Qt platform theme, GTK via gsettings/`settings.ini`), not
  just Forest's own processes.
- **Notification `ActivationToken`.** Spec 1.2 signal sent before
  `ActionInvoked` so the client can raise its window via xdg-activation.
  Without it, clicking e.g. a browser notification may not focus the browser.
  Needs a way to get an xdg-activation token from Qt for the popup's surface.
- **More notification hints.** `body-markup` (`FadingLabel`'s clipped path
  paints plain text, so it needs rich-text rendering first), `urgency`
  (styling; critical shouldn't time out), `image-data` / `image-path`,
  `transient`, `category`/`desktop-entry` (better app icon/name lookup).
- **More hardware info on the About page** (#61). Machine vendor/model,
  GPU, root-disk capacity, BIOS/firmware, boot mode, Secure Boot state,
  motherboard. Maybe a separate "more details" page (lshw-gtk-like).
- **Night light setting.** Displays page control for screen colour
  temperature (warm tint at night, like wlsunset): on/off, day/night
  temperatures, schedule (sunrise/sunset or fixed times). Biome serves
  `wlr-gamma-control-unstable-v1`; Forest needs a small client (own daemon or
  a managed `wlsunset`). Scope beyond temperature (brightness, gamma) TBD.
- **Theme CSS consolidation** ([detail](review-2026-11.md#theme-css)). Shared rules into
  `base`, colours via `palette()`, icon sizes via `qproperty`. Do before the
  accent colour; Biome regenerates `builtin.qss` after.
- **Custom accent colour.** Let the user override the theme's `highlight`
  palette colour from the settings UI (Themes page). Hardcoded greens in
  theme CSS need to move to `palette(highlight)` first.

## 0.13.0 — hardware & system

Pairs with Biome 0.5.0 (data-control, for the clipboard manager).

- **Devices / mount manager panel plugin** (#37). Panel icon to
  mount/unmount/eject removable drives, plus remounting chosen drives at
  login. UDisks2 directly or via Solid; decide together with the battery
  monitor's backend (#38) so both use the same one.
- **UPower battery backend** ([detail](review-2026-11.md#battery-backend)). Event-driven
  level and warnings; settle #38's thresholds against it.
- **libpulse volume backend** ([detail](review-2026-11.md#volume-backend)). Works through
  PipeWire, follows the default sink; volume keys move to services-app.
- **Battery monitor: configurable low-battery notifications** (#38).
  Thresholds and actions (warn / suspend / hibernate) in settings.
- **Native brightness control.** The shipped brightness hotkeys shell out to
  `bash -c "brightnessctl …"`. Add a services D-Bus method (e.g. logind
  `Session.SetBrightness` on the backlight device, no setuid helper) and make
  it a built-in hotkey action; the lock-screen hotkeys item wants it too.
- **Hotkeys on the lock screen.** Biome hotkeys are off while locked, so
  volume/brightness keys do nothing there. Add a per-hotkey
  `allow_on_lockscreen` flag (visible in hotkey settings, default on for the
  shipped volume/brightness entries) and have `forest-lockscreen` run only
  flagged hotkeys. Opt-in, since a remapped key could otherwise run arbitrary
  commands unauthenticated. Parse actions with `library/hotkeyconfig`.
- **Shared monitor base** ([detail](review-2026-11.md#graph-monitor)). One `GraphMonitor`
  for cpu/memory, one colour format. Before #28.
- **System monitor popups** (#28). Popups for CPU/memory monitors with
  per-core and RAM/swap graphs, and per-sensor graphs for `sensors`.
- **Clipboard manager.** Clipboard history; needs Biome's data-control
  support (Biome 0.5.0).
- **Deliberate `org.forest` API** ([detail](review-2026-11.md#forest-dbus)). Scriptable
  slots only, interface names, shared constants, async calls.
- **Forest-owned portal config** ([detail](review-2026-11.md#portal-config)). Enables
  screen sharing via xdg-desktop-portal-wlr. Pairs with Biome 0.5.0.

## 0.14.0 — panel

- **Multi-instance applet API** ([detail](review-2026-11.md#applet-api-v4)). Applet factory,
  applet names in `Panel.conf`, "Add applet", host-owned context menus.
  Prerequisite for multiple panels.
- **Main menu app index** ([detail](review-2026-11.md#app-index)). XDG desktop-ID precedence,
  parse once, debounced. Prerequisite for the combined taskbar.
- **Panel tooltips.** Themed tooltips on panel items (e.g. app name on
  quicklaunch hover). Needs QSS styling to match the theme and custom
  positioning so they open away from the panel edge, not over it.
- **Directory menu.** Panel plugin that browses a folder as cascading menus.
  Unblocked: panel menus are QMenus with real submenus.
- **Hot corners.** Small layer-shell surfaces in the screen corners (a
  client can't see the global pointer under Wayland), each with a
  user-configurable action (show desktop, open menu, run command, task view
  once it exists).
- **Multiple panels** (#53). E.g. dock at the bottom + status bar at the
  top. `Panel.conf` and panel settings assume a single panel today.
- **Vertical panels.** Left/right screen edges. Some plugins carry
  untested `TopToBottom` layout branches; most assume a horizontal panel.
- **Combined taskbar and quicklaunch** (#55, needs research). Pinned
  launchers that become window buttons when running (KDE icons-only task
  manager / Plank style). The hard part is matching toplevel `app_id`s to
  `.desktop` files reliably; windowlist's `iconresolver` is a starting point.

## Later (unscheduled)

- **Network manager plugin.** The old `nmcontrol` stub is being removed
  (0.11.2); a real plugin would start fresh. Needs connection status, Wi-Fi
  list/connect, wired/VPN toggles. Pick a backend first (NetworkManagerQt vs.
  raw `org.freedesktop.NetworkManager` D-Bus). Low priority: `nm-applet`
  works fine as is.
- **Task view.** Overview of open windows (and workspaces) with live
  thumbnails. Per-toplevel capture is available (`ToplevelCapture` in
  `library/toplevels`, used by the windowlist previews).
- **Desktop widgets** (#54). Windows 7-style gadgets (analog clock, resource
  monitor, weather) on a layer-shell surface, with a panel button to
  show/hide them.
- **Session save/restore** (needs research). Relaunch the apps open at
  logout. No standard Wayland protocol for this yet (session-management is
  still experimental), so the fallback is relaunching `.desktop` entries
  matched from foreign-toplevel `app_id`s, without window positions or app state.
- **Greeter theme.** The `greetd` user has no `Forest.conf`, so
  `forest-greeter` always uses the default theme. Store the theme choice in
  `/etc/forest/Login.conf` (like the login wallpaper) and write it through
  `forest-login-helper` and its polkit action.
- **Per-user lockscreen wallpaper.** The lockscreen always matches the
  greeter's system-wide login wallpaper. Decide whether users should be able
  to override it (it would have to live in the user's config, not
  `/var/lib/forest/login`).
- **Laptop lid switch.** Display profiles handle docking, but "lid closed ⇒
  disable the internal panel" needs logind lid events (`LidClosed`) feeding
  the displays service. Wait for a laptop target.
- **Workspaces beyond Biome's model** (decoupling). deskswitch/windowlist
  assume one ext-workspace group (a single active workspace); a compositor
  with per-output groups would need per-group handling. The window ->
  workspace mapping and "move to desktop" still go through the Biome-only
  `org.biome.Workspaces` (`BiomeWorkspaces`, hidden when absent) — replace it
  with a standard protocol if one appears (ext-workspace has no toplevel
  membership).
- **Hotkey-opened popups get keys only where popup activation is honored.**
  They can't grab (no press serial), so `popup.h` shows them non-grabbing
  and activates them with the GlobalShortcuts `activation_token`. Biome
  focuses an activated popup; sway, KWin and Hyprland seem to activate only
  toplevels, so there the main menu opens without keyboard. Portable
  alternative: popups as their own `exclusive` layer surfaces. Revisit if
  other compositors matter.
- **Package management page in forest-settings.** A "Packages"/"Apps" page
  over PackageKit (`libpackagekitqt6`), a native Wayland alternative to
  root-run GUI package managers (GNOME Software is the stopgap). Browse/search,
  install/remove, and updates may outgrow a single settings page.
- **Library and CMake cleanup** ([detail](review-2026-11.md#library-cleanup)). Wayland
  extension teardown, break up `miscutills`, CMake helpers. Best before the
  applet API change.
- **Record lists as QSettings arrays** ([detail](review-2026-11.md#record-lists)). Hotkeys,
  autostart, quicklaunch, panel.
- **Session services outside the applet process** (needs decision;
  [detail](review-2026-11.md#services-process)).
- **Fingerprint unlock** (low priority). The lockscreen runs one PAM stack;
  `pam_fprintd` blocks typing until it times out. Parallel password and
  fingerprint conversations, as GDM does.
