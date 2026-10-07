# Display Settings — Implementation Plan

Roadmap item: 0.10.0 "Display settings plugin". Multi-session work: each
phase below is one branch off `develop` and one PR, sized for roughly one
session. Update this doc as phases land: delete finished checklist items,
record decisions that changed. Remove the plan doc (or fold the settled parts
into `development-notes.md`) once the feature ships.

## Goals

- Configure outputs live: enable/disable, mode (resolution + refresh), scale,
  position, rotation/flip, primary screen.
- **Display profiles:** named layouts, persisted by Forest, applied through
  `wlr-output-management-unstable-v1`. They replace hand-written `wlr-randr`
  scripts (`~/.screenlayout/two.sh`/`three.sh` bound to Meta+2/Meta+3) and the
  swap-`Biome.conf`-and-relog workflow.
- **Profile hotkeys** that work through the normal hotkey system and are
  intuitive to set up from either the display page or the hotkeys page.
- Compositor-agnostic: only the standard protocol plus Forest's own D-Bus.
  Works on Biome, sway, Hyprland, etc.

## Decisions

### Protocol binding: hand-bind with `qtwaylandscanner`, not libkscreen

Checked 2026-10-06 against Trixie's `libkf6screen8` 6.3.4. Its dependencies
are only Qt (no KF6 frameworks), so the dependency cost would have been fine.
But it has **no wlroots backend**. The installed backends are `KSC_KWayland`
(KWin's private `kde_output_device_v2` / `kde_output_configuration_v2`
only), `KSC_XRandR`, `KSC_QScreen` (read-only) and `KSC_Fake`. The "reuse
libkscreen's wlr-output-management backend" note in `biome/docs/history.md`
was wrong (now corrected there). KScreen on wlroots compositors was never merged upstream (lxqt
uses kanshi/wlr-randr there instead).

So Forest binds the protocol itself with
`qt6_generate_wayland_protocol_client_sources`, the same way
`library/toplevels` binds foreign-toplevel and ext-workspace. The protocol is
small: manager, head, mode, configuration, configuration_head.

### Architecture: daemon in services-app, editor in system-settings

```
system-settings plugin (forest-settings)        services-app (forest process)
  DisplayPage  ── reads heads ──┐                 displays service
  (editor, profile UI)          │                   /org/forest/displays (D-Bus)
       │                        ▼                   owns Displays.conf (sole writer)
       │              library/outputs  ◄──────────  applies profiles: login, hotplug,
       └── D-Bus: saveProfile/applyLayout/...        hotkeys; confirm/revert overlay
                                                     writes display/primary_screen
```

- **`library/outputs`** (new static lib): protocol client plus profile model.
  It's used by both processes.
- **`displays` service in `services-app`**, next to hotkeys/notifications
  (decided: no new process). It is long-lived, so it owns everything that
  must happen without the settings app open: apply at login, hotplug
  auto-pick, hotkey-triggered switching, and the revert timer.
- **System settings → Displays page** in `system/system-settings`. It's an
  editor: it reads live heads through `library/outputs` (read-only, plus
  `test` requests for instant validation). All applies and all profile
  writes go through the daemon over D-Bus.
- **The daemon is the only writer of `Displays.conf`.** This avoids
  two-process QSettings races. It also means a confirmed layout gets saved
  even if the settings app crashed or was closed during the confirm
  countdown.

### Profile model: there is always an active profile

Decided: no separate "current layout" concept. Every applied layout belongs to
a profile.

- First Apply on a monitor set with no matching profile auto-creates one,
  named from its outputs (e.g. "DP-2 + HDMI-A-1"; the user can rename it).
- Apply in the editor updates the active profile. "Save as new profile…"
  forks it.
- Users who never think about profiles still get their settings kept across
  logins, because Biome never persists applied changes (`Biome.conf` is
  startup-only, see Biome `architecture-notes.md` "Live output
  management").

### Matching (kanshi-style auto-pick)

Decided: auto-pick on hotplug and at login.

- A profile records **every connected output at save time**, including
  disabled ones. Its *output set* is the set of identity keys.
- A profile matches when its output set equals the connected set exactly.
  Among matches, the most recently used (`last_used`) wins. That's why
  "two" and "three" (same three monitors connected, DP-1 off in "two") don't
  fight: the last one you picked sticks.
- No match → leave the compositor's layout alone. The page then shows
  "Unsaved setup", and the first Apply creates a profile.
- **Output identity key:** `make|model|serial` when the head has a non-empty
  serial that is unique among connected heads. Otherwise fall back to the
  connector `name` (identical monitors without serials, some laptop
  panels). Store the connector name either way; Qt's `QScreen::name()` and
  `display/primary_screen` use it.
- **Mode matching on apply:** exact `width×height@refresh_mHz`. Otherwise the
  same size at the closest refresh. Otherwise the head's preferred mode
  (logged).
- `nextProfile()` cycles through the matching profiles in a stable order
  (by name).

### Biome remembers the last applied layout

Decided 2026-10-06. Layout changes are expected to come through the protocol,
not through hand edits to `Biome.conf`. Biome persists every successful
protocol `apply` (from any client, `wlr-randr` included) and restores it at
startup. This is generic compositor behaviour, not a Forest shortcut.
**Forest still never writes `Biome.conf` itself.**

- After a successful `apply`, Biome writes `[Outputs]` in the user
  `~/.config/Biome/Biome.conf` for every head in that configuration:
  `enabled`, `mode` (`WxH@Hz`, never `preferred`), `scale`, `x`, `y`,
  `transform`. It uses the same keys `core/output_config.cpp` already reads.
- `test`, hotplug repair (`output_relayout()`) and startup layout never
  write. Connectors not in the applied configuration keep their entries, so
  an undocked monitor's last settings survive.
- `conf.d` drop-ins are never touched. The user file wins per key, so the
  remembered layout overrides shipped defaults.
- Hand edits still work as a startup default until the next protocol apply
  overwrites them. Caveat: `QSettings` rewrites the whole file, so comments
  in `Biome.conf` are lost on the first write.
- Keyed by connector (the existing schema). Identity-based matching stays
  Forest's job (profiles).

Forest still applies the matching profile at login, but skips it when the
live state already equals the profile. Biome's restored layout is normally
Forest's last apply, so there's no second modeset unless the monitor set
changed while logged out.

### Hotkeys: both entry points, one storage

Decided: profiles get shortcuts from both the Displays page and the Hotkeys
page.

- Storage is unchanged: a normal `[hotkeys]` entry in `Forest.conf`. That
  keeps one source of truth, one conflict check and one portal binding path.
- **New: a string argument on D-Bus actions.** Add `arg=<value>` to the
  `DBUS:` action format. `globalhotkey` passes it as a single string
  argument. Profile ids are `[a-z0-9-]` (uuid without braces), so the
  existing comma/`=` parsing is safe.
  - Switch: `DBUS:bus=Session,service=org.forest,path=/org/forest/displays,method=applyProfile,arg=<id>`
  - Cycle: `…,method=nextProfile`
- Actions reference the **profile id, not its name**, so renames don't break
  them. On rename the daemon also rewrites the hotkey `description`
  ("Display profile: <name>"). Deleting a profile removes its hotkeys.
- **Hotkeys page:** fill in the currently empty "Built-in" action list
  (`builtindbusLwidget` in `edithotkeywidget.ui`). Entries are one "Display
  profile: <name>" per profile (via `DisplayProfiles::load()`, linking
  `library/outputs`), plus "Next display profile" since `nextProfile()`
  already exists and it's one fixed list entry. A built-in compiles down to
  the `DBUS:` string above. When loading an entry, a `DBUS:` action that
  matches a known built-in shows as that built-in, not as raw custom D-Bus
  fields. Compare parsed fields, not strings: the writer emits keys in
  `QHash` order. This is generic, so other built-ins (show menu, show
  desktop, lock) can move here later.
- **Displays page:** a "Shortcut: [Meta+3]" button on the active profile,
  using the same key-capture widget (pause/resume hotkeys while capturing).
  It writes and updates the `[hotkeys]` entry whose action targets that
  profile id, then calls `reloadhotkeys`.
- Extract the hotkey config read/write (`HotkeyData` ⇄ `Forest.conf`, the
  `DBUS:` parsing) into a small shared library (`library/hotkeyconfig`).
  Today the format is parsed in `foresthotkeys::loadhotkeys()` and written in
  `services-settings/hotkeys`. Both the Displays page and those two need it,
  and so does the daemon (rename/delete). The roadmap's "Hotkeys on the lock
  screen" item wants the same sharing. It also holds the next-free
  `item-NNNN` id (`HotkeySettings::add_item` crashes on an empty
  `[hotkeys]` today), lookup by key sequence for the conflict warning, and
  the key-capture button (moved out of `edithotkeywidget`).
- The daemon and `foresthotkeys` share `services-app`: wire a
  `Displays::hotkeysChanged` signal to `reloadhotkeys` in `services.cpp`
  rather than a D-Bus call to itself.
- Capturing a shortcut that's already bound warns and offers to reassign it
  (needed anyway once two pages write hotkeys).

### Confirm/revert, owned by the daemon

- Applying an **edited** layout that changes enable/mode/scale/transform
  shows a confirmation card on every enabled output: "Keep these display
  settings? Reverting in 15 s" [Revert] [Keep]. The daemon draws it (layer
  shell, overlay layer, `KeyboardInteractivityExclusive` on the primary
  screen's card; Escape = revert, Enter = keep). It doesn't depend on the
  settings app's window, which may be on a screen that just went dark.
- Revert happens on timeout, button, Escape, or a daemon restart: the
  profile isn't written until Keep, so startup re-applies the last confirmed
  one.
- No confirmation for position-only or primary-only edits, or for applying
  a saved profile (hotkey, profile selector, auto-pick). Saved profiles were
  confirmed when they were saved.

### Primary screen

- Part of the profile. On apply, the daemon writes `display/primary_screen`
  in `Forest.conf`. That's the value `ScreenTracker::primary()` already
  reads, so consumers don't change.
- Edited through `applyLayout`, whose JSON becomes
  `{"outputs":[…],"primary":"DP-1"}`. A primary-only change needs no confirm
  (`needsConfirm` ignores it).
- Live switching needs one addition. Today a primary change with no geometry
  change isn't noticed (`GeometryManager::handle_geometry_change` only runs
  on screen changes). The daemon emits D-Bus `primaryChanged` when
  `display/primary_screen` actually changes, and `ScreenTracker` forwards it
  as `primary_changed()`: it remembers the last primary and emits only on a
  difference, and skips the 2 s screen debounce when the screens themselves
  aren't changing, so the panel doesn't lag behind Apply.
- Consumers: panel → `handle_geometry_change` (already rebuilds on a primary
  mismatch), desktop → `handleScreenChange`, lockscreen → `placeCard`.
  Logout doesn't follow live: it's a short-lived dialog that picks the
  primary when it opens.

## Protocol notes (`wlr-output-management-unstable-v1`)

Biome is on wlroots 0.18 → manager v4 (make/model/serial from v2,
`adaptive_sync` v4). Copy the XML from
`misc/wlroots` (`git show 0.18.2:protocol/wlr-output-management-unstable-v1.xml`)
into `forest/protocol/`. Wayland-protocols doesn't ship wlr protocols.

- Heads/modes stream in, then `manager.done(serial)`. Treat the state as
  consistent only after `done`. Build configurations against the latest
  serial. `configuration.cancelled` = stale serial: refresh and retry once.
- `head.finished` on unplug. Hotplug arrives as new heads, then `done`. The
  daemon debounces (reuse `RunOnce`) before auto-picking.
- A configuration must mention every head (`enable_head` or
  `disable_head`). Always send the full layout.
- Disabled heads have no `current_mode`. Biome substitutes the current or
  preferred mode, but other compositors may not, so always send an explicit
  mode when enabling.
- Effective (logical) size = mode size / scale, width/height swapped for
  90/270 transforms. Use wlroots' rounding (`wlr_output_effective_resolution`)
  so the editor's gap math matches the compositor's.
- **Biome rejects layouts with gaps** (`layout_is_connected()`; overlap is
  allowed, for mirroring). The editor must never produce a gap: snap on drag,
  normalize to (0,0), and `test` before Apply. Treat a `failed` test as an
  inline error, not a dialog.
- Qt side: Forest's own `layer-shell-qt` surfaces already follow screen
  changes via `ScreenTracker`. Nothing new is needed there beyond the
  primary signal.

## Storage: `~/.config/Forest/Displays.conf`

`QSettings("Forest", "Displays")`, separate from `Forest.conf` like
`Panel.conf`/`Quicklaunch.conf`. The daemon is the only writer.

```ini
[General]
active=3f2c…            ; profile id last applied and confirmed

[profiles]
3f2c…\name=Three monitors
3f2c…\last_used=2026-10-06T12:00:00
3f2c…\primary=DP-1
3f2c…\outputs\size=3
3f2c…\outputs\1\key=Dell Inc.|DELL U2723QE|ABC123
3f2c…\outputs\1\connector=DP-1
3f2c…\outputs\1\enabled=true
3f2c…\outputs\1\mode=3840x2160@60000
3f2c…\outputs\1\scale=1.5
3f2c…\outputs\1\x=0
3f2c…\outputs\1\y=0
3f2c…\outputs\1\transform=normal
3f2c…\outputs\1\adaptive_sync=false
```

## D-Bus: `org.forest` `/org/forest/displays`

Registered with `ExportAllSlots | ExportAllSignals`, interface
`org.forest.displays` (`Q_CLASSINFO`, so `busctl`/`gdbus` calls don't need
Qt's `local.Displays`). Lower-case slot names. Payloads are JSON strings
(simple, and this is a Forest-internal interface).

| Slot / signal | Purpose |
|---|---|
| `applyProfile(QString id)` | apply a saved profile, no confirm; ignored (logged) if it doesn't match the connected set |
| `nextProfile()` | cycle matching profiles |
| `applyLayout(QString json)` | apply an edited layout + primary for the active profile (or a new one), with confirm/revert |
| `saveProfileAs(QString name, QString json) → id` | fork: an edited layout is applied with confirm/revert and becomes the new profile on Keep; an unedited one is snapshotted |
| `renameProfile(id, name)`, `deleteProfile(id)` | also update/remove the profile's hotkeys; deleting the active one clears `active` |
| `saveCurrentAsProfile(QString name) → id` | snapshot the live layout (phase 1's no-UI path) |
| `identify()` | show the identify overlay |
| `profilesChanged`, `activeProfileChanged(id)`, `primaryChanged(name)` | signals for the editor and `ScreenTracker` |
| `keepLayout()`, `revertLayout()` | answer a pending confirmation (the card calls the same code) |
| `confirmPending`, `layoutKept`, `layoutReverted`, `applyFailed` | confirm-card lifecycle, for the editor |

## UI: System settings → Displays

- **Profile bar** (replaces the header label): active profile selector, with
  entries that match the connected monitors on top; the rest under "Other
  setups" (not applicable; rename/delete only). Picking a matching profile
  discards unsaved edits and calls `applyProfile` (no confirm). Rename
  (inline), Delete (asks first), "Save as new profile…" (no separate New:
  there's always an active profile, so they'd be the same), and the
  Shortcut button.
- **Primary display:** a combo box of the enabled outputs, page-level rather
  than a per-output checkbox (which couldn't be unchecked).
- **Arrangement canvas** (`QWidget` with one `#DisplaysOutput` button per
  enabled output): sized by effective size and scaled to fit, labelled with
  name + model. Drag to move. Snap to the edges/centres of the others. On
  release, normalize the top-left to (0,0) and enforce connectivity. Disabled
  outputs sit in a strip below the canvas. Click to select.
- **Selected output:** Enabled, Resolution, Refresh rate, Scale (presets
  100–300 % in 25 % steps plus custom), Orientation (Normal / 90 / 180 / 270,
  plus Flipped variants).
- **Buttons:** Identify (connector + model on each screen for ~3 s: a
  pass-through overlay card from the daemon, next to `ConfirmCard`), Revert
  (discard edits), Apply.
- The page refreshes from the protocol on `done` and from daemon signals, so
  a hotkey switch while the page is open shows up straight away.
- Styling via `settings.css` object names, like the other system-settings
  pages.

## Phases

Each phase ends in something the user can manually test (keyboard/visual
testing is the user's, per the usual workflow).

Phase 1 (backend, no UI: `library/outputs`, the `displays` service,
`DBUS:` `arg=`, Biome persisting applies) and phase 2 (Displays page,
confirm/revert apply) are done.

Phase 3 is split into two PRs; 3b builds on 3a's rename/delete. Neither
needs Biome changes.

### Phase 3a — profiles, primary, identify

- [ ] Daemon: `renameProfile`, `deleteProfile`, `saveProfileAs`,
      `identify`, `primaryChanged`; `applyLayout` JSON carries `primary`.
- [ ] Profile bar: selector with "Other setups", rename, delete, "Save as
      new profile…".
- [ ] Primary display combo; `ScreenTracker::primary_changed()`;
      panel/desktop/lockscreen follow it live.
- [ ] Identify overlay.

### Phase 3b — hotkeys

- [ ] `library/hotkeyconfig` extraction, used by `foresthotkeys`, the
      Hotkeys page and the daemon.
- [ ] Built-in action list on the Hotkeys page.
- [ ] Shortcut button on the Displays page, with conflict warning.
- [ ] Daemon keeps profile hotkeys in step on rename/delete.

### Phase 4 — hardening and wrap-up

- [ ] Edge cases: identical monitors without serials, an output unplugged
      mid-confirm, saved mode no longer offered, all outputs disabled
      (refuse), mirroring (overlap) shown sensibly on the canvas.
- [ ] Try on another wlroots compositor (sway) to check decoupling.
- [ ] Changelog entry, remove the roadmap item, fold durable notes into
      `development-notes.md`, drop or trim this plan.

## Open questions

- **Lid switch / laptop panel.** Out of scope until there's a laptop target.
  The matching model handles docking, but "lid closed ⇒ disable eDP" needs
  logind lid events.

## Dev testing without extra monitors

Headless Biome works (fake heads, one custom mode each: fine for canvas,
matching and confirm logic, not mode lists). Override `XDG_CONFIG_HOME`, or
its applies overwrite the real `Biome.conf`:

```sh
XDG_CONFIG_HOME=/tmp/biome-cfg WLR_BACKENDS=headless WLR_HEADLESS_OUTPUTS=2 \
  WLR_RENDERER=pixman WLR_LIBINPUT_NO_DEVICES=1 biome -s <client>
```

It logs its `WAYLAND_DISPLAY`; point `wlr-randr` at that.
