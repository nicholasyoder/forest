# Forest Greeter — Architecture

**Stack:** greetd + cage + Qt Widgets (pure C++/QSS, no QML). Implemented in
`greeter/greeter-app/`; ships as the separate `forest-greeter` .deb (depends
on `forest`, `greetd`, `cage` — see `debian/control`).

greetd launches `forest-greeter` inside `cage` (a minimal single-app Wayland
compositor) on VT1. After authentication, greetd starts the selected
session (`startforest-wayland` for the Wayland/Biome session, or an X11
`xsessions` entry) and kills the greeter. Qt Widgets renders identically
under `cage`'s Wayland QPA — same raster pipeline, fonts, and QSS as the
rest of the desktop.

Why greetd (not LightDM/SDDM): LightDM is unmaintained with no Wayland
support; SDDM is Qt6/QML-capable but its own greeter process still starts
an embedded X server. greetd is a minimal daemon with no greeter opinion at
all — you own the whole login UI and its IPC — which fit a Wayland-native
Forest best. See `greetd-ipc(7)` for the daemon's own protocol reference if
the IPC layer ever needs revisiting.

## Components (`greeter/greeter-app/`)

- **`GreetdClient`** — wraps the greetd Unix-socket IPC (`$GREETD_SOCK`,
  4-byte length-prefixed JSON) over `QLocalSocket`. Drives
  `create_session`/`post_auth_message_response`/`start_session`/
  `cancel_session`, and surfaces `authMessage`/`authSucceeded`/`authFailed`
  signals back to the UI.
- **`UserListModel`** — parses `/etc/passwd`, filtered to UID >= 1000 with a
  valid login shell. Per-user: display name (GECOS), home dir, face icon
  (`~/.face` or `/var/lib/AccountsService/users/<username>`).
- **`SessionListModel`** — reads `*.desktop` entries from
  `/usr/share/xsessions/` and `/usr/share/wayland-sessions/` (the latter
  added for the Phase 5 Biome cutover — see `biome/docs/history.md`), tagged
  with which directory they came from (`SessionType::X11`/`Wayland`). Forest's
  own session is Wayland-only post-cutover, but other software's `xsessions`
  entries (xfce, i3, bspwm, ...) still show up here.
- **`GreeterWindow`** — fullscreen login UI: user/session selectors,
  password field, clock, shutdown/reboot buttons
  (`systemctl poweroff`/`reboot`), drives the auth flow through
  `GreetdClient`'s signals.

## Theming

Uses the normal `fstyleloader::loadstyle("greeter")` mechanism — a
`greeter.css` per theme layer, same inheritance chain as every other
component. One wrinkle specific to the greeter: it runs as the `greeter`
system user before any login, so `QSettings("Forest","Forest")` reads from
`/var/lib/greeter/` (set via `HOME=` in `etc/greetd/config.toml`), and that
user's config must have `theme=` set explicitly or it falls back to the
default (`Round-Dark`).

## Startup sequence

```
greetd (VT1) → cage -s -- forest-greeter → user authenticates
  → forest-greeter sends start_session, exits → cage exits (-s flag)
  → greetd launches the selected session's Exec= (e.g. startforest-wayland)
```

## X11 sessions

greetd has no opinion on X vs Wayland — it just `exec`s whatever `start_session`
argv it's given, with no `DISPLAY` and no X server. That's fine for
`wayland-sessions` entries, but an `xsessions` entry (Forest itself doesn't
ship one anymore, but other installed software's WMs/DEs might) needs an X
server bootstrapped first, the way a traditional display manager's own
`Xsession` script would do it.

`GreeterWindow::onAuthSucceeded()` handles this by prefixing the session's
`Exec=` with `/usr/share/forest/forest-greeter-xsession` when
`SessionListModel` tagged it as `SessionType::X11`. That script resolves the
client to an absolute path (`startx` silently falls back to `xterm` for a
bare name it can't look up) and runs:

```
startx <resolved-client> -- -seat "${XDG_SEAT:-seat0}" -keeptty vt${XDG_VTNR}
```

`-seat`/`vtN` get Xorg GPU and VT access through logind instead of needing
root — the VT number must be passed explicitly since Xorg only skips its
(root-only) `/dev/tty0` probe when one's given on the command line.
`XDG_SEAT`/`XDG_VTNR` come from the PAM session greetd opens, same as
`XDG_SESSION_TYPE`. Requires `xinit` (`forest-greeter` Recommends it,
along with `xserver-xorg`) — without it the wrapper fails with a clear
"startx not found" instead of a silent hang.
