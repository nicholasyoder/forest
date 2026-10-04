# Session Locker — Plan

Roadmap item for 0.9.0 (#5): lock the session and power displays off on
idle, replacing the interim `swayidle` + `wlopm` setup. Animated screensavers
are out of scope.

## What Biome provides

- **`ext-session-lock-v1`** — a lock client that dies without
  `unlock_and_destroy` leaves the session locked (red blank rect); a new
  client may `lock()` again to take over. While locked, no compositor or
  portal hotkey fires except VT switch. Clicking a lock surface gives it
  keyboard focus. See `biome/docs/architecture-notes.md` ("Session lock").
- **`ext-idle-notify-v1`** + **`idle-inhibit-unstable-v1`** — inhibitors only
  count while their surface is visible.
- **`wlr-output-power-management-unstable-v1`** — Biome no longer wakes
  outputs on input; the client must power them on at idle `resumed`.

## Architecture

Two processes, like swayidle + swaylock: the lock UI can crash without
leaving the session stuck locked or the displays dark.

### `forest-lockscreen` (`locker/lockscreen/`)

One-shot: locks, authenticates, unlocks, exits. Usable standalone.

- **Session-lock shell integration** (`library/sessionlock`). Qt has no
  `ext-session-lock-v1` support, so this is a QtWaylandClient shell
  integration (`QWaylandShellIntegrationTemplate` + `QWaylandShellSurface`),
  set per window the way layer-shell-qt does
  (`misc/layer-shell-qt/src/interfaces/window.cpp`). Sends `lock()`, creates a
  lock surface per `QScreen` (including hotplugged ones), maps `configure` to
  a resize + `ack_configure`, and calls `unlock_and_destroy` on success.
  `finished` without `locked` means another locker holds the lock: exit
  non-zero. Uses Qt private API (`Qt6::WaylandClientPrivate`,
  `qt6-wayland-private-dev`), so it needs a rebuild on every Qt minor bump,
  the same as layer-shell-qt.
- **UI** — one window per screen: wallpaper and clock everywhere; the
  password card on the primary screen only (`ScreenTracker::primary()`), as
  in the greeter. Following the pointer can come later. No popups (a lock
  surface can't parent any).
- **`PamAuth`** — PAM in a `QThread` with a real conversation function
  (handles multiple and non-secret prompts), service `forest-locker`
  (`etc/pam.d/forest-locker`: `@include common-auth`). Non-root password
  checks go through `unix_chkpwd`. Clear the password buffer after use.
- **Readiness** — prints `locked` on stdout once the compositor sends
  `locked`, so the daemon knows the screen is actually covered.
- **Debug safety hatch** — Debug builds only: `--unlock-after <sec>`. A
  stuck lock can also be taken over by running `swaylock` against Biome's
  socket from another VT.

### `forest-locker` (`locker/locker-daemon/`)

Long-running daemon with no windows (apart from the dim overlay), started by
`forest-session` next to `forest`.

- **Idle** — one `ext_idle_notification_v1` per threshold: dim warning,
  display off (and lock, if enabled), and a shorter display-off delay while
  locked. At
  `resumed`, power every output on and drop the dim overlay.
- **Output power** — `zwlr_output_power_v1` per screen; get each `wl_output`
  from its `QScreen` through the native interface.
- **Lock supervision** — spawns `forest-lockscreen` and waits for `locked`. If
  it exits non-zero while the session should be locked, respawn it (Biome
  lets the replacement take over); back off on repeated crashes.
- **logind** (system bus):
  - `Lock` → lock; `Unlock` → unlock (terminates the lockscreen).
  - `PrepareForSleep(true)` → lock while holding a `delay` sleep inhibitor
    taken at startup; release it after `locked`, then take it again on
    `PrepareForSleep(false)`.
  - `SetLockedHint` / `SetIdleHint` on the session object.
  - These signals arrive on the real session path, not `session/self`:
    resolve the session ID first.
- **`org.freedesktop.ScreenSaver`** (session bus) — `Inhibit` / `UnInhibit`
  (track the caller's bus name and drop its cookies when it vanishes),
  `Lock`, `GetActive`. Firefox, Chromium, mpv and VLC often use this rather
  than the Wayland inhibit protocol. While any cookie is held, ignore the
  idle thresholds.
- **Dim warning** — `layeroverlay::showOnAllScreens` with a translucent
  black overlay and its own namespace (`forest-locker-dim`) a few seconds
  before display-off. Give it an
  empty input region so the waking click goes through. It fades only once
  Biome's fade config lists that namespace (the shipped-config blocker on
  the roadmap); otherwise it appears abruptly.
- **Settings** — `QSettings("Forest", "Locker")`. Watch the file with
  `QFileSystemWatcher` and re-arm the idle notifications on change.

### Triggers

Standard paths only, no `org.forest.Locker`: other lockers work with
Forest's triggers and Forest's locker works with other tools.

- Meta+L hotkey → `loginctl lock-session`.
- Logout dialog **Lock** button → login1 `Session.Lock`.
- `xdg-screensaver lock` / apps → `org.freedesktop.ScreenSaver.Lock`.

### Settings (`locker/locker-settings/`)

A new top-level "Lock Screen" settings plugin (reorganising the settings
categories is a separate, later job). Needs a `plug-0005` `name=locker`,
`settings-only=true` entry in `etc/forest/Forest.conf`, also appended by
`upgrade_0_9_0()`.

- Turn displays off after N minutes (`display_off_minutes`, default 10,
  matching the old stopgap; 0 = never).
- Lock when the displays turn off (`lock_on_display_off`, default on).
- Dim 10 s before display-off (`dim_before_display_off`, default on;
  skipped when display-off is 10 s or less).
- Lock on suspend (`lock_on_suspend`, default on).
- Displays off after N minutes idle while locked
  (`locked_display_off_minutes`, default 1; 0 = never) — for manual locks,
  so a locked screen doesn't stay lit for the full display-off delay.

The daemon reads minutes as doubles, so fractional values work for testing.

## Shared UI with the greeter

The lock screen should look like the greeter (same card, clock, avatar and
wallpaper treatment) without the user, session or power controls.

**Now:**

- Move the clock + wallpaper background, the circular-avatar helper and the
  face-icon lookup into a small `library/loginui` static lib used by both.
  Make the background **per screen** (one widget per `QScreen`) rather than
  the greeter's single cage-spanning surface; the greeter can keep wrapping
  it for now.
- The lockscreen loads `fstyleloader::loadstyle("greeter")` and uses the
  greeter's `greeter_*` object names for the shared widgets: identical
  styling with no new theme CSS. Rename to shared names later, if wanted.
- The lockscreen gets its own small password card, not the greeter's
  `PasswordView`. That widget is tied to greetd (manual-entry, session
  button, back button), and splitting it is more work than the card itself.

**Later** (a separate refactor): move the greeter from cage to Biome,
one window per output on the per-screen background; split `PasswordView`
into a shared card; maybe rename `greeter.css` to a shared component.

The greeter reads the greeter user's `Forest.conf` (`/var/lib/greeter`), and
the lockscreen reads the logged-in user's. Same code, so the lock screen
shows the user's own wallpaper.

## Phases

1. **Lockscreen** — `library/sessionlock`, `library/loginui` (extracted from
   the greeter, which keeps working), `forest-lockscreen` with PAM, the
   pam.d file, and build-deps (`libpam0g-dev`, `qt6-wayland-private-dev`).
   Tested by running it by hand.
2. **Daemon** — idle + output power, lockscreen supervision, logind, started
   from `forest-session`. Delete the interim
   `~/.config/autostart/biome-dpms-interim.desktop` after this.
3. **Integration** — settings plugin, dim warning, `org.freedesktop.ScreenSaver`,
   Meta+L default, Lock button in the logout dialog, `upgrade_0_9_0()`
   entries, packaging (`debian/control`, `forest.install`), and updates to
   both roadmaps (Biome's "swayidle + wlopm stand in" line goes away).
4. **Polish** (optional for 0.9.0) — caps-lock indicator. Volume and
   brightness keys while locked moved to the roadmap ("Hotkeys on the lock
   screen"): they need a per-hotkey opt-in, not hardcoded actions.

Live logind calls (`Inhibit`, `SetLockedHint`, `Session.Lock`) in phases 2–3
must be confirmed with the user before running them, because one such call
once crashed logind.
