# Forest Greeter — Implementation Plan

**Stack:** greetd + cage + Qt Widgets (pure C++/QSS, no QML)

---

## Overview

A new `greeter/` top-level subdirectory produces a single executable `forest-greeter`.
greetd launches it inside `cage` (a minimal single-app Wayland compositor) on VT1.
After authentication, greetd kills the greeter, starts the X11 Forest session.
Qt Widgets renders identically on Wayland via the Wayland QPA — same raster pipeline,
same fonts, same QSS as the rest of the desktop.

---

## System Dependencies

Two packages must be installed on the target system (not build-time deps):

| Package | Purpose |
|---------|---------|
| `greetd` | Display manager daemon, replaces lightdm |
| `cage` | Minimal Wayland compositor to host the greeter UI |

Build-time: no new libraries needed — the greetd IPC is a plain Unix socket with
length-prefixed JSON, implemented directly in C++.

---

## Files to Create

```
greeter/
├── CMakeLists.txt                        # add_subdirectory(greeter-app)
└── greeter-app/
    ├── CMakeLists.txt
    ├── main.cpp
    ├── greetdclient.h / .cpp             # greetd IPC
    ├── greeterwindow.h / .cpp            # fullscreen login UI
    ├── userlistmodel.h / .cpp            # parses /etc/passwd
    └── sessionlistmodel.h / .cpp         # parses xsessions/ and wayland-sessions/ *.desktop

usr/share/forest/themes/base/greeter.css  # new CSS layer (inherited by all themes)

etc/greetd/
└── config.toml                           # greetd daemon config
```

---

## Files to Modify

| File | Change |
|------|--------|
| `CMakeLists.txt` (root) | Add `add_subdirectory(greeter)` and install rule for `etc/greetd/` |
| `usr/share/forest/themes/base-dark/greeter.css` | Dark color overrides (optional) |
| `usr/share/forest/themes/base-light/greeter.css` | Light color overrides (optional) |

---

## greetd IPC Protocol

The socket path is in `$GREETD_SOCK`. Messages are **4-byte little-endian length** +
**JSON body**. `GreetdClient` handles all framing.

### Client → greetd

```json
// 1. Start auth for a user
{ "type": "create_session", "username": "alice" }

// 2. Respond to a PAM prompt (password, 2FA code, etc.)
{ "type": "post_auth_message_response", "response": "hunter2" }

// 3. Launch the session after success
{ "type": "start_session", "cmd": ["/usr/share/forest/startforest-wayland"], "env": [] }

// Cancel and reset (e.g. user switches username mid-auth)
{ "type": "cancel_session" }
```

### greetd → Client

```json
{ "type": "success" }
{ "type": "error", "error_type": "auth_error", "description": "..." }
{ "type": "auth_message", "auth_message_type": "secret",  "auth_message": "Password: " }
{ "type": "auth_message", "auth_message_type": "visible", "auth_message": "..." }
{ "type": "auth_message", "auth_message_type": "info",    "auth_message": "..." }
{ "type": "auth_message", "auth_message_type": "error",   "auth_message": "..." }
```

`auth_message_type`:
- `secret` — hide input (password field)
- `visible` — show input (username confirmation, TOTP, etc.)
- `info` / `error` — display only, no input needed; send `post_auth_message_response` with `null` response to continue

### Auth Flow

```
create_session(username)
  → auth_message{secret, "Password: "}
    post_auth_message_response("hunter2")
      → success
        start_session(["/usr/share/forest/startforest-wayland"])
        [greeter exits]
```

---

## Class Responsibilities

### `GreetdClient` (QObject)

Wraps the Unix socket IPC. Runs the socket read/write on Qt's event loop
(use `QLocalSocket` — it speaks Unix sockets and integrates with the event loop).

**Public slots:**
- `createSession(const QString &username)`
- `postAuthResponse(const QString &response)` — pass empty string for info/error prompts
- `startSession(const QString &sessionExec)`
- `cancelSession()`

**Signals:**
- `authMessage(const QString &type, const QString &message)` — type is the raw `auth_message_type`
- `authSucceeded()`
- `authFailed(const QString &description)`

### `UserListModel` (QAbstractListModel or plain struct list)

Parses `/etc/passwd` on construction. Filters to UID >= 1000 with a valid login shell
(i.e. shell is not `/usr/sbin/nologin` or `/bin/false`).

Per user, stores:
- `username` — login name
- `displayName` — GECOS field (first comma-separated segment), falls back to username
- `homeDir`
- `faceIconPath` — checks `~/.face` first, then `/var/lib/AccountsService/users/<username>`

### `SessionListModel` (plain struct list)

Reads all `*.desktop` files in `/usr/share/xsessions/` and
`/usr/share/wayland-sessions/` (added for the Phase 5 Biome cutover — see
`biome/docs/plan.md`). For each, parses:
- `Name=` → display name
- `Exec=` → command passed to `start_session`

### `GreeterWindow` (QWidget)

Fullscreen, frameless, always-on-top widget. Covers all screens.

Responsibilities:
- Loads wallpaper (reads path from `QSettings("Forest","Forest")`, key `wallpaper/file`)
  and scales it via `miscutills::get_wallpaper_scaled()`
- Applies stylesheet via `fstyleloader::loadstyle("greeter")`
- Shows a centered login card containing:
  - User selector (QComboBox populated from `UserListModel`)
  - Password field (QLineEdit with `EchoMode::Password`)
  - Session selector (QComboBox populated from `SessionListModel`)
  - Login button
  - Error/info message label (hidden when empty)
  - Clock label (updated by a 1-second QTimer)
  - Shutdown / reboot buttons (call `systemctl poweroff` / `systemctl reboot`)
- Connects to `GreetdClient` signals and drives the auth flow:
  - On show: calls `createSession` with the initially selected user
  - On user change: calls `cancelSession`, then `createSession` with new user
  - On login button / Enter: calls `postAuthResponse` with the password field text
  - On `authSucceeded`: calls `startSession` with selected session's Exec
  - On `authFailed`: shows error text, clears password field, re-focuses it
  - On `authMessage{info|error}`: displays message, auto-continues with null response

---

## Theme / Styling

The greeter uses the same `fstyleloader` mechanism as every other component.
`fstyleloader::loadstyle("greeter")` walks the parent_themes chain and loads
`greeter.css` from each layer.

**Important:** the greeter runs as the `greeter` system user before any login, so
`QSettings("Forest","Forest")` reads from that user's home (e.g. `/var/lib/greeter/`).
The `greeter` user's `~/.config/Forest/Forest.conf` must have `theme=` set, or
`fstyleloader` will fall back to its default (`Round-Dark`). The greetd `config.toml`
can set `HOME=/var/lib/greeter` to point at the right config directory.

Add `greeter.css` to `usr/share/forest/themes/base/` to establish base styles
(card background, input field sizing, etc.). Theme variants (`base-dark`, `base-light`,
`base-rounded`, `base-circle`) add their color/radius overrides exactly as they do for
`logout.css`.

---

## CMake

### `greeter/greeter-app/CMakeLists.txt`

```cmake
add_executable(forest-greeter
    main.cpp
    greetdclient.cpp
    greeterwindow.cpp
    userlistmodel.cpp
    sessionlistmodel.cpp
    greetdclient.h
    greeterwindow.h
    userlistmodel.h
    sessionlistmodel.h
)

set_target_properties(forest-greeter PROPERTIES CXX_STANDARD 17)

target_link_libraries(forest-greeter PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)
forest_link_flogger(forest-greeter)
forest_link_miscutills(forest-greeter)

install(TARGETS forest-greeter RUNTIME DESTINATION usr/bin COMPONENT greeter)
```

(`Qt6::Network` is needed for `QLocalSocket`.)

### Root `CMakeLists.txt` additions

```cmake
add_subdirectory(greeter)
install(DIRECTORY etc/greetd/ DESTINATION /etc/greetd COMPONENT greeter)
```

The `COMPONENT greeter` tag marks which install rules belong to the greeter — debhelper uses
`debian/forest-greeter.install` to distribute those files into the separate package.

---

## Debian Packaging

The greeter ships as a **separate `.deb`** (`forest-greeter`) built from the same source package
as `forest`. This is the standard Debian multi-binary-package pattern: one `dpkg-buildpackage`
run produces both `.deb` files.

### Dependency rationale

- `forest` does **not** depend on `forest-greeter` — you can use Forest without swapping your
  display manager.
- `forest-greeter` depends on `forest` because it calls `fstyleloader::loadstyle("greeter")`,
  which reads CSS from `/usr/share/forest/themes/` (installed by the `forest` package).
- `forest-greeter` also depends on `greetd` and `cage` (runtime requirements).

### Files to add / modify

| File | Action |
|------|--------|
| `debian/control` | Add `Package: forest-greeter` stanza; add `qt6-base-private-dev` or network headers to `Build-Depends` for `QLocalSocket` |
| `debian/forest-greeter.install` | New file — lists paths to pull into the greeter package |
| `debian/forest.install` | New file — explicitly lists all non-greeter paths (or leave absent and let debhelper auto-assign the remainder) |

### `debian/control` addition

```
Package: forest-greeter
Architecture: amd64
Depends: ${shlibs:Depends}, ${misc:Depends}, forest, greetd, cage
Description: Forest greeter for greetd
 A Qt-based login greeter for the greetd display manager,
 styled with the Forest theme system. Runs inside the cage
 Wayland compositor on VT1.
```

Also add `libqt6network6-dev` (or the appropriate Qt6 network dev package) to the
top-level `Build-Depends`.

### `debian/forest-greeter.install`

```
usr/bin/forest-greeter
etc/greetd/
usr/share/forest/themes/*/greeter.css
```

This captures the binary, the greetd config, and all per-theme greeter CSS files.

---

## System Configuration

### `etc/greetd/config.toml`

```toml
[terminal]
vt = 1

[default_session]
command = "cage -s -- /usr/bin/forest-greeter"
user = "greeter"
```

The `greeter` user is created automatically by the `greetd` package.
`cage -s` stops the compositor once the greeter exits (the `-s` flag).

---

## Startup Sequence

```
Boot
 └─ greetd starts on VT1
     └─ launches: cage -s -- forest-greeter
         └─ cage starts a Wayland compositor
             └─ forest-greeter starts, connects to cage via WAYLAND_DISPLAY
                 └─ user authenticates
                     └─ forest-greeter sends start_session → exits
                         └─ cage exits (due to -s flag)
                             └─ greetd launches: /usr/share/forest/startforest-wayland
                                 └─ startforest-wayland sets up env, execs biome -s forest-session
```
