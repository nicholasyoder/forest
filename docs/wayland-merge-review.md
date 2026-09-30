# Wayland branch review (pre-merge into develop)

Review of `git diff develop...wayland` (28 commits, ~3.5k lines added excluding
the xfwm4 theme deletions), done 2026-09-26. Work through the phases below
across sessions; tick items off (`[x]`) as they land and note the commit.

Some items need manual verification by the user (behavioural/visual) — marked
**(manual test)**.

---

## Phase 1 — Session environment & startup cleanup (small, low risk) — done, 25da548

- [x] **1.1 Drop `QT_QPA_PLATFORM` entirely.** `usr/share/forest/startforest-wayland`
  exports `QT_QPA_PLATFORM=wayland`, then `forest/main.cpp`,
  `session/session-app/main.cpp`, `logout/logout-app/main.cpp` and
  `settings/main.cpp` each `qunsetenv` it. Unneeded: Qt 6 picks wayland from
  `XDG_SESSION_TYPE=wayland` (already exported). Only the session-app unset has
  any effect (everything else descends from it), and it doesn't stop the leak
  anyway: `dbus-update-activation-environment --systemd --all` runs after the
  export, so every D-Bus/systemd-activated app still gets it. Remove the export
  and all four unsets + comments. **(manual test: Qt apps still come up native
  Wayland; FreeCAD-style apps can pick xcb.)**
- [x] **1.2 Push `WAYLAND_DISPLAY` into the activation environment.**
  `dbus-update-activation-environment` runs before Biome starts, so
  `WAYLAND_DISPLAY` doesn't exist yet; nothing in Biome pushes it later
  (grepped). D-Bus/systemd-activated services (portal backends, D-Bus-launched
  apps) get no display. Run `dbus-update-activation-environment --systemd
  WAYLAND_DISPLAY` from `forest-session` (first process that has it).
  **(manual test: `systemctl --user show-environment` shows it.)**
- [x] **1.3 Delete the no-op `useLayerShell()` + `qunsetenv("QT_WAYLAND_SHELL_INTEGRATION")`
  pair** in `forest/main.cpp`. The comment itself says the call is a no-op for
  this process after `QApplication`, and the next line removes the env var it
  sets. Delete both calls and the ~20-line comment.
- [x] **1.4 Remove per-target `CXX_STANDARD 11` pins.** Top-level CMake sets 17;
  10 targets still pin 11 (`forest`, `forest-logout`, `panel-app`, `clock`,
  `batterymonitor`, `mainmenu`, `nmcontrol`, `services-app`, `desktop-app`, ...).
  Also drop the explicit 17 pins and the C++17 explanation comments in
  `panel/panel-plugins/{deskswitch,windowlist}/CMakeLists.txt`.
- [x] **1.5 Packaging/doc nits.**
  - `docs/roadmap.md` still says Forest "can't `Depends: biome`" — stale since
    commit 65f708b added it. Remove that item.
  - `libx11-dev` in `debian/control` Build-Depends no longer needed directly
    (`libxcursor-dev` pulls it in).
  - `startforest-wayland`: gtk-3.0 block reuses the `GTK2_CONF_FILE` variable
    name; mixed tab/space indentation.

## Phase 2 — Panel popups (`panel/panel-library/popup.h`) — done

- [x] **2.1 Mouse-anchored policies were lost.** `CenteredOnMouse` /
  `EdgeAlignedOnMouse` are ignored by the new `positionOnLauncher()`. Windowlist's
  right-click menu uses `CenteredOnMouse` (`windowlist.cpp:30`), and since
  windowlist is the stretch widget the menu now centres on the whole taskbar.
  Fix: for `*OnMouse`, use a 1px-wide anchor rect at
  `launcherwidget->mapFromGlobal(QCursor::pos()).x()` (Qt still knows the pointer
  position within its own surface). **(manual test)**
- [x] **2.2 Anchor rect relative to the wrong widget.** It's computed against the
  `objectName == "panel"` widget, but the xdg positioner is relative to the
  toplevel (`launcherwidget->window()`, i.e. GeometryManager's shell). Only works
  because the shell layout has zero margins. Map to `window()` instead; delete
  `getpanelwidget()`.
- [x] **2.3 Nested event loop in `positionOnLauncher()`.** The `processEvents`
  wait for the parent to be exposed can delete `launcherwidget` (already guarded)
  *and* `this` (not guarded). Replace with deferring `show()` until the
  toplevel's Expose event (event filter on the toplevel `QWindow`, or a
  `QPointer<popup>` guard as a minimum).
- [x] **2.4 App-wide event filter always installed.** Each popup does
  `qApp->installEventFilter(this)` in its constructor and keeps it while hidden.
  Install in `showpopup()`, remove on hide/close.
- [x] **2.5 Condense comments** (see Phase 7 — popup.h has ~80 lines of comments,
  the constructor block alone is 22).
- [x] **2.6 Found during testing** (window preview, `windowlist/imagepopup.cpp`):
  - The expose wait needs the old 500ms cap: QtWayland reports a visible
    panel unexposed after any frame-callback timeout, so an uncapped wait
    could hang forever. (Why Biome withholds frame callbacks from a visible
    panel is still open — investigate on the Biome side.)
  - Preview never auto-closed: `QCursor::pos()` goes stale once the pointer
    leaves our surfaces; use `underMouse()`.
  - Preview reopened at 10x10 after a click-off close: imagepopup's
    `resize(10,10)` while hidden stuck. `finishShow()` now `adjustSize()`s;
    the resize hacks are gone.

## Phase 3 — Screen tracking & primary screen — done

Root cause of panel vs. icons/logout landing on different outputs: nothing chose
a screen explicitly. The panel's fresh shell got whatever screen Qt assigned a
new top-level; icons/logout used `qApp->primaryScreen()` (first output Qt saw).

- [x] **3.1–3.3** `ScreenTracker` (miscutills): debounced, tracks
  `QList<QPointer<QScreen>>`, watches every screen's `geometryChanged`, emits
  `screens_replaced()` / `geometry_changed()`. Replaces the duplicated checks in
  `GeometryManager` and `desktop`. Also fixed `RunOnce` leaking a `QTimer` per fire.
- [x] **Primary screen.** `ScreenTracker::primary()`: `display/primary_screen`
  (output name, e.g. `DP-1`) in `~/.config/Forest/Forest.conf`, else the
  top-left screen (min x, then y). Hand-edited for now; UI comes with the
  display settings plugin. Panel, desktop icons and logout dialog pin to it;
  the panel rebuilds if it changes on a geometry change.
- [x] **6.3 (moved here)** Logout dialog: `setScreen(primary())`, anchors/margins dropped.
- [x] **3.4** Dropped `setFixedSize(screen->size())` on all-edge-anchored
  surfaces (LayerShellQt sends 0 for doubly-anchored dims). The wallpaper still
  needs an initial `resize(screen->size())`: with the icons layout it's
  otherwise 0x0 and Qt never maps it. `layeroverlay`/`wallpaperwidget` now
  take their `QScreen*` (first half of 6.4).
- [x] Desktop "Select all" was bound to the first `iconswidget`, dead after any
  screen change.
- [x] **(manual test)** after `three.sh`: panel/icons/logout on DP-1; live
  layout switch; monitor sleep/wake; logout centred on the scaled 4K;
  wallpaper fills each screen behind the panel.

## Phase 4 — Windowlist, deskswitch & Wayland object lifetimes — done

- [x] **4.1 Plugin reload leaks bound protocol objects.** Managers are parented
  to the plugin and own their handles; the plugin destructor calls `release()`
  (`stop`, then destroy the proxy and self-delete on `finished` — the generated
  `*_finished()` handlers are empty and none of the three has a destructor
  request). Safe because panel plugins are never unloaded.
- [x] **4.2 Shared `org.biome.Workspaces` client.** `BiomeWorkspaces`
  (panel-library): cached map, async `GetWindowWorkspaces`/`MoveToplevelToWorkspace`.
- [x] **4.3 Button creation / icon resolution.** Button created on the first
  `done`; identifier lives on `ForeignToplevelHandle` (pairing can finish
  first); icon re-resolved only when app_id changes.
- [x] **4.4 Group lifetime.** Groups kept in a list and destroyed on `removed`.
  Multi-group *semantics* (one active workspace per group) → `docs/roadmap.md`.
- [x] **4.5** `BiomeWorkspaces::isAvailable()` is a sync bus-daemon check at
  construction (Biome owns `org.biome` before forking the session). Without it,
  ext-foreign-toplevel-list isn't bound, nothing is paired, and "Move to
  desktop" is hidden. Late binding isn't an option: wlroots replays toplevels
  newest-first, so a late ext replay wouldn't line up with the wlr queue.
- [x] **(manual test)** Reload the panel (panel settings) several times, then
  open/close windows: no crash, taskbar/deskswitch correct. `WAYLAND_DEBUG=1`
  shows `stop` → `finished` per manager. Window buttons still filter per
  desktop; "Move to desktop" works; icons correct for terminals/browsers.

## Phase 5 — Hotkeys (GlobalShortcuts portal)

- [ ] **5.1 Request/Response race.** `GlobalShortcutsPortal::awaitResponse()`
  subscribes to `org.freedesktop.portal.Request::Response` only *after*
  `CreateSession`/`BindShortcuts` returns. Biome auto-accepts instantly, so the
  Response can be emitted before the match rule exists — hotkeys silently never
  bind. Per the portal spec: compute the request path
  (`/org/freedesktop/portal/desktop/request/<escapedSender>/<handle_token>`) and
  subscribe *before* the call. `escapedSender()` already exists for this.
- [ ] **5.2 Make portal calls async** (`QDBusMessage::createMethodCall` +
  `asyncCall`). Currently `QDBusInterface` (blocking introspection) + blocking
  `call()` against a D-Bus-activated service that can be slow at session start,
  blocking the whole `forest` process (panel + desktop).
- [ ] **5.3 `showdesktop` is a warning stub** — remove the slot/any default
  binding, or add a roadmap item.
- [ ] **5.4 `reloadhotkeys()` while paused** rebinds keys during hotkey capture
  in settings — check whether that path can happen.
- [ ] **5.5** `hotkey::exec()`: the session/system bus branches are identical except
  the connection (pre-existing) — collapse; use `asyncCall`.

## Phase 6 — Remaining async D-Bus + unported/coupled components

- [ ] **6.1 Tray (`panel/panel-plugins/systray/trayicon.cpp`) is fully
  synchronous.** Each item builds a `QDBusInterface` (blocking introspection) and
  does 3–4 blocking property reads per `NewIcon`. One hung tray app freezes the
  panel for up to 25s. Switch to one async `org.freedesktop.DBus.Properties.GetAll`
  per change signal. Also: `QIcon::setThemeSearchPaths` is appended to globally
  per item; SNI `Passive` status is ignored (spec says hide).
- [ ] **6.2 Notification popups not ported.**
  `services/services-app/notifications/notifypopup.cpp:89-92` still positions a
  plain frameless toplevel with `move()` — a no-op for xdg toplevels. It lands
  wherever Biome places it, appears in the windowlist, can take focus. Make it a
  layer surface (`LayerOverlay`/`LayerTop`, anchored bottom-right with margins,
  keyboard interactivity none) — `layeroverlay`/`wallpaperwidget` show the
  pattern. **(manual test)**
- [x] **6.3 Logout dialog centering** — done in Phase 3.
- [ ] **6.4 Layer overlay per-screen loop duplicated 3×** (`forest/forest.cpp`,
  `logout.cpp` constructor and `start_action()`). `QScreen*` ctor arg added in
  Phase 3; a static `showOnAllScreens(...)` could still collapse the loops. Rename/merge `logoutmanager::startbackfade()` — it no
  longer fades anything.
- [ ] **6.5 Cursor settings** (`system/system-settings/cursorthemesettings.cpp`):
  `org.biome.Cursor.SetTheme` is a blocking bespoke call. Decide whether Biome
  should instead watch `~/.icons/default/index.theme` or the portal Settings
  `cursor-theme` key (decoupling goal). Also check whether GTK apps pick up the
  change (startforest-wayland hardcodes Adwaita into gtk-3.0 `settings.ini`).
- [ ] **6.6 Greeter** `GreetdClient::startSession` splits Exec on spaces
  (pre-existing, more visible with the xsession wrapper) — use
  `QProcess::splitCommand`.

## Phase 7 — Comment pass

Project rule: comments 1–3 lines, non-obvious fact only; history belongs in
commit messages. Also remove references to Biome file paths / "Workstream D" /
session logs that will go stale. Do this per file alongside the phase that
touches it where possible; whatever's left, here.

- [x] `panel/panel-library/popup.h` — ~80 lines; every block → 1–2 lines. (Phase 2)
- [ ] `panel/panel-plugins/systray/trayicon.cpp` — ADL note, empty-list signature
  note, raw `Properties.Get` rationale, `showTrayMenu` history (~60 lines).
- [ ] `services/services-app/systemtray/statusnotifierwatcher.h` — 17-line block →
  "QDBusContext only works on the object passed to registerObject(), not an adaptor."
- [x] `panel/panel-plugins/windowlist/extforeigntoplevelhandle.h` (`m_readySent`),
  `windowlist.h` (pending queues), `windowlist.cpp` (`tryPairPendingHandles`,
  `onWindowAdded`), `extforeigntoplevellist.h`. (Phase 4)
- [x] `panel/panel-app/geometrymanager.{h,cpp}`, `desktop/desktop-app/wallpaperwidget.cpp`
  (exclusive zone), `desktop/desktop-app/desktop.cpp` (`handleScreenChange`). (Phase 3)
- [ ] `logout/logout-app/logout.cpp` (`startbackfade`, `start_action`, `cancel`).
- [ ] `services/services-app/hotkeys/foresthotkeys.cpp` (`pauseHotkeys`),
  `globalshortcutsportal.{h,cpp}`, `hotkey.h`, `keysym_table.h` header note.
- [x] `panel/panel-library/extworkspace{manager,handle}.{h,cpp}`,
  `panel/panel-plugins/deskswitch/deskswitch.h`. (Phase 4)
- [ ] `mainmenu.cpp` (outsideclicked).
- [x] `panel/panel-library/CMakeLists.txt`, `panel/panel-plugins/windowlist/CMakeLists.txt`
  (protocol vendoring notes). (Phase 4)
- [x] `panel/panel-plugins/windowlist/imagepopup.h`, `windowbutton.{h,cpp}` —
  drop "matches the old X11 menu" notes. (Phase 4)
- [ ] `forest/forest.cpp` startup overlay comment.
- [ ] Docs: `docs/qmenu-migration-plan.md` is mostly resolved-bug narrative — trim
  to what's still actionable (Task 1); trim the history paragraph at the end of
  `docs/development-notes.md`'s "Debugging Wayland" section.

---

## Suggested merge gate

Before merging into develop: Phase 1, 2.1, 3.1–3.2, 4.1, 5.1, 6.2, 6.3, and
Phase 7. The async D-Bus work (5.2, 6.1) and the 6.5 cursor decision can follow
on develop.
