# Greeter / Display Manager Research

## Display Manager Comparison

### LightDM — Original candidate, ruled out

- **Qt support:** Qt5 only. No Qt6 path. Qt6 GitHub issue has received zero maintainer response.
- **Maintenance:** Effectively abandoned. Original Canonical maintainer stated in 2022 it is "essentially unmaintained." Canonical dropped it as Ubuntu's default in 2017.
- **Wayland:** None. A Wayland PR has been open and unmerged for six years.
- **Custom greeter:** Library-based (`liblightdm-gobject-dev` C API or `liblightdm-qt5-3-dev`). Writing a greeter requires a compiled binary.
- **Verdict:** Dead end. Wrong Qt version, no Wayland, no maintainers.

---

### SDDM — Recommended

- **Qt support:** Full Qt5 and Qt6. Qt6 support landed in 0.20 (June 2023), solidified in 0.21 (February 2024). Themes declare `QtVersion=6` in `metadata.desktop` and get the Qt6 greeter. KDE is on a Qt6-only trajectory, making SDDM's Qt6 path permanent.
- **Maintenance:** Actively maintained by KDE, LXQt, and Liri teams. KDE uses SDDM as its official display manager and has strong incentive to keep it healthy.
- **Wayland:** SDDM launches Wayland user sessions perfectly. The greeter itself running on Wayland is experimental as of 0.21 (resolving in 1.0). Currently, SDDM starts a brief embedded X server for the login screen, then launches the Wayland session.
- **Custom greeter:** Themes are pure QML — just a directory with `metadata.desktop` and `Main.qml`. No compiled greeter binary needed. The QML environment provides:
  - `sddm` — proxy for login, shutdown, suspend, hibernate
  - `userModel`, `sessionModel`, `screenModel` — data models
  - Test mode: `sddm-greeter-qt6 --test-mode --theme /path/to/theme`
- **Dependencies:** Qt5 or Qt6, PAM, optional systemd/logind. No GTK. Xorg needed for the greeter itself currently.
- **Verdict:** Best fit. Qt6 QML themes means the login screen is just another Qt Quick UI — same tooling and language as the rest of the desktop. Large existing theme ecosystem for reference.

---

### greetd — Best Wayland-native option

- **Qt support:** greetd itself is a minimal Rust daemon with no Qt dependency. Available Qt6 greeters:
  - **QtGreet** — C++/Qt6, Wayland-native, most established
  - **qmlgreet** — QML/Qt6, uses `ext-session-lock` Wayland protocol
- **Maintenance:** greetd daemon is actively maintained. Individual Qt6 greeters vary in maturity. Slated to replace LightDM in openSUSE Leap 16.
- **Wayland:** Excellent. Built for the Wayland era. Qt6 greeters use `ext-session-lock` or `wlr-layer-shell` natively.
- **Custom greeter:** greetd communicates via a simple JSON IPC protocol over a Unix socket (`greetd-ipc(7)`). The greeter is any process that speaks this protocol — write it in Qt6/QML/C++, total freedom. The cost: you implement the full PAM conversation loop (username, password, MFA) yourself. No pre-built QML components provided.
- **Dependencies:** Extremely minimal daemon (Rust binary + PAM). Weight comes from your greeter.
- **Verdict:** Most future-proof for a fully Wayland-native DE. More work than SDDM — you own the entire greeter.

---

### GDM — Ruled out

GNOME Shell is the greeter. GTK/GJS only. Heavy GNOME stack dependency. No Qt path exists.

---

## Recommendation

**Use SDDM.** The login screen is written as a pure QML theme — same language and tooling as the rest of the desktop. Qt6 support is production-ready and KDE-backed.

If a fully Wayland-native greeter (no embedded X server) is a hard requirement from day one, use **greetd + a custom Qt6/QML greeter** instead. The IPC protocol is simple enough to implement in a few hundred lines.

A pragmatic path: start with SDDM, migrate to greetd if SDDM's Wayland greeter becomes a blocker.

---

## SDDM Theme QML API Reference

```qml
// Context objects provided by sddm-greeter-qt6:

// Login / power actions
sddm.login(user, password, session)
sddm.shutdown()
sddm.reboot()
sddm.suspend()
sddm.hibernate()
sddm.canShutdown()   // bool
sddm.canReboot()     // bool
sddm.canSuspend()    // bool
sddm.canHibernate()  // bool

// Signals
sddm.loginSucceeded()
sddm.loginFailed()

// Data models (Qt list models)
userModel       // roles: name, realName, homeDir, icon, loggedIn, needsPassword
sessionModel    // roles: name, file, comment (key used in sddm.login)
screenModel     // roles: name, geometry, primary
```

**Theme structure:**
```
themes/forest-greeter/
├── metadata.desktop
└── Main.qml
```

**`metadata.desktop`:**
```ini
[SddmGreeterTheme]
Name=Forest Greeter
Description=Forest desktop login screen
Author=Nicholas Yoder
Type=sddm-theme
Version=1.0
Website=
Screenshot=preview.png
MainScript=Main.qml
ConfigFile=theme.conf
QtVersion=6
```

**Install path:** `/usr/share/sddm/themes/forest-greeter/`

**LightDM config** (`/etc/sddm.conf`):
```ini
[Theme]
Current=forest-greeter
```

**Test during development:**
```bash
sddm-greeter-qt6 --test-mode --theme /path/to/themes/forest-greeter
```

---

## Implementation Plan

See [greeter-plan.md](greeter-plan.md) for the full implementation plan (greetd + cage + Qt Widgets).

## LightDM Plan (archived — superseded by SDDM, then greetd)

Originally researched using LightDM with `liblightdm-gobject-dev` (the C/GObject API,
since the Qt library is Qt5-only). The C API would have been wrapped in a thin Qt6 class.
Abandoned in favor of SDDM after discovering LightDM is effectively unmaintained and has
no Wayland support.

Key packages that would have been needed:
- `liblightdm-gobject-dev` — C API headers
- `libglib2.0-dev` — GLib dependency
- Greeter `.desktop` file in `/usr/share/xgreeters/`
