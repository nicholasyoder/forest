# Session Locker Implementation Plan

## Architecture Overview

A new standalone app (`forest-locker`) under `session/locker/`, started by `forest-session` at login alongside the WM. It handles everything: idle detection, DPMS control, and the lock screen UI. A companion settings page gets added to the existing `session-settings` plugin.

---

## New Dependencies

| Library | Package | Purpose |
|---|---|---|
| `libxss` | `libxss-dev` | `XScreenSaverQueryInfo()` — idle time in ms |
| `libpam` | `libpam0g-dev` | PAM authentication for unlock |
| `libxext` (DPMS) | already installed | `DPMSForceLevel()` — blank/off monitor |
| `Qt6::DBus` | already in project | logind integration |

---

## Components

### `forest-locker` app (`session/locker/locker-app/`)

**`IdleMonitor`** — polls `XScreenSaverQueryInfo()` every 5 seconds via `QTimer`. Fires signals at configurable thresholds (read from `QSettings("Forest", "Locker")`):
- `blank_screen` → `DPMSForceLevel(dpy, DPMSModeSuspend)`
- `display_off` → `DPMSForceLevel(dpy, DPMSModeOff)`
- `lock` → emits `lock()` signal
- On activity return → `DPMSForceLevel(dpy, DPMSModeOn)`
- Updates logind `SetIdleHint(true/false)` via system D-Bus

**`LockScreen`** — one `QWidget` per screen with `Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint`. Black background, centered password prompt. On show: `XGrabKeyboard` + `XGrabPointer` (prevents bypass). Uses `QLineEdit` in password mode.

**`PamAuth`** — runs PAM in a `QThread` (non-blocking UI). Wraps `pam_start` / `pam_authenticate` / `pam_end` with service name `forest-locker`. Emits `authenticated()` or `failed(reason)`.

**`LockerApp`** — the main controller:
- Connects to **system bus** `org.freedesktop.login1` session object, listens for `Lock` / `Unlock` signals
- Listens for `PrepareForSleep` (auto-lock on suspend)
- Registers `org.forest.Locker` on **session bus** with `lock()` / `unlock()` D-Bus methods (so hotkeys or other tools can trigger it)
- Calls `SetLockedHint(true/false)` on logind to sync state

### Settings page (added to `session-settings` plugin)

New `ScreensaverSettings` class with:
- Enable idle timers (checkbox)
- Blank screen after N minutes (spinbox)
- Turn off display after N minutes (spinbox)
- Lock screen after N minutes (spinbox)
- Lock on suspend (checkbox)

---

## Files to Create / Modify

**New:**
```
session/locker/CMakeLists.txt
session/locker/locker-app/CMakeLists.txt
session/locker/locker-app/main.cpp
session/locker/locker-app/lockerapp.{h,cpp}
session/locker/locker-app/idlemonitor.{h,cpp}
session/locker/locker-app/lockscreen.{h,cpp}
session/locker/locker-app/pamauth.{h,cpp}
session/session-settings/screensaver/screensaversettings.{h,cpp}
etc/pam.d/forest-locker
```

**Modified:**
```
session/CMakeLists.txt                              ← add_subdirectory(locker)
session/session-app/sessionapp.cpp                  ← startProcess("forest-locker")
session/session-settings/CMakeLists.txt             ← add screensaver sources
session/session-settings/sessionsettings.{h,cpp}   ← add ScreensaverSettings
CMakeLists.txt (root)                               ← find libxss + libpam
debian/control                                      ← add build-deps
```

---

## Key Design Decisions

1. **Idle detection via `libxss`** (not pure Qt): XScreenSaver extension reports true X11 idle time including input from all devices, not just the app's own event stream. This is the standard approach (same as `xautolock`, `light-locker`).

2. **DPMS via Xlib** (not `xset` subprocess): Direct API call is cleaner and faster than shelling out. `dpms.h` is already present on the system.

3. **PAM in a thread**: Authentication can block; running it off the UI thread keeps the lock screen responsive (user sees a "verifying..." state).

4. **Separate locker app** (not a `forest` plugin): The locker must survive `forest` crashing and has different privilege concerns (PAM, keyboard grab). A standalone process is more robust and correct for a session locker.

5. **logind `Lock` signal integration**: Means `loginctl lock-session` and tools like systemd's suspend-on-lid work out of the box without needing a separate trigger mechanism.
