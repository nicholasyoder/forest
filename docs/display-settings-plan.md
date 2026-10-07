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
- **Profile hotkeys** that work through the normal hotkey system, set up from
  the Hotkeys page.
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

### Profile model: Apply applies, Save saves

Decided 2026-10-07 (replaces "there is always an active profile"). Applying
and saving are separate actions:

- **Apply never writes a profile.** An edited layout is applied (with
  confirm/revert) and becomes an unsaved layout. This allows one-off setups
  (e.g. a projector) without touching the profile you started from.
- **Save** writes the editor's layout either over an existing profile (keeping
  its id, name and hotkeys) or as a new profile. It never applies anything.
  To change a profile, the user edits, applies, checks the result, and then
  saves over the original.
- **Active profile** = the profile whose saved layout equals the live one.
  After an applied edit is kept, the daemon checks the matching profiles. If
  one equals the applied layout, it becomes active. Otherwise none is active.
  Saving a layout that equals the live one also makes that profile active.
- Unsaved layouts survive relogin because Biome remembers the last applied
  layout (below), not because of profiles. Auto-pick still switches a
  monitor set that has a profile back to that profile on hotplug or login. So
  an unsaved tweak to a profiled monitor set is temporary, but a layout for
  a monitor set with no profile is left alone.
- Saving over a profile with a different output set is allowed. The profile
  then matches the new set.

### Matching (kanshi-style auto-pick)

Decided: auto-pick on hotplug and at login.

- A profile records **every connected output at save time**, including
  disabled ones. Its *output set* is the set of identity keys.
- A profile matches when its output set equals the connected set exactly.
  Among matches, the most recently used (`last_used`) wins. That's why
  "two" and "three" (same three monitors connected, DP-1 off in "two") don't
  fight: the last one you picked sticks.
- No match → leave the compositor's layout alone. The page then shows the
  unsaved "Current layout".
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

### Hotkeys: set up on the Hotkeys page only

Decided 2026-10-07 (replaces "both entry points"): profiles get shortcuts
only from the Hotkeys page's built-in action list. A shortcut button on the
Displays page would have meant sharing the key-capture widget and hotkey
writing across two settings plugins, plus a conflict check, for one button.
The Displays page gets a plain text note pointing to Services → Hotkeys
instead. A clickable link would need cross-plugin navigation in the settings
shell (Hotkeys is nested under Services), which doesn't exist yet.

- Storage is unchanged: a normal `[hotkeys]` entry in `Forest.conf`. That
  keeps one source of truth and one portal binding path.
- **New: a string argument on D-Bus actions.** Add `arg=<value>` to the
  `DBUS:` action format. `globalhotkey` passes it as a single string
  argument. Profile ids are `[a-z0-9-]` (uuid without braces), so the
  existing comma/`=` parsing is safe.
  - Switch: `DBUS:bus=Session,service=org.forest,path=/org/forest/displays,interface=org.forest.displays,method=applyProfile,arg=<id>`
  - Cycle: `…,method=nextProfile`
- Actions reference the **profile id, not its name**, so renames don't break
  them. On rename the daemon also rewrites the hotkey `description`
  ("Display profile: <name>") if it's still the default for the old name, so
  a description the user wrote is kept. Deleting a profile removes its
  hotkeys.
- **Hotkeys page:** fill in the currently empty "Built-in" action list
  (`builtindbusLwidget` in `edithotkeywidget.ui`). Entries are one "Display
  profile: <name>" per profile (via `DisplayProfiles::load()`, linking
  `library/outputs`), plus "Next display profile" since `nextProfile()`
  already exists and it's one fixed list entry. A built-in compiles down to
  the `DBUS:` string above. When loading an entry, a `DBUS:` action that
  matches a known built-in shows as that built-in, not as raw custom D-Bus
  fields. Compare parsed fields, not strings: older entries were written in
  `QHash` key order, and an empty interface matches any. This is generic, so other built-ins (show menu, show
  desktop, lock) can move here later.
- **Displays page:** a plain text note at the bottom: profiles can be given
  shortcuts under Services → Hotkeys.
- Share only the `DBUS:` action parse/format (a plain struct ⇄ string), in a
  small `library/hotkeyconfig`. Today it's parsed in
  `foresthotkeys::loadhotkeys()` and `HotkeySettingItem::edit()`, written in
  `HotkeySettingItem::save()`, and the daemon (rename/delete) would make a
  third copy. The format writer emits keys in a fixed order. No key-capture
  widget or `HotkeyData` UI types move. The roadmap's "Hotkeys on the lock
  screen" item can grow it later.
- Fix `HotkeySettings::add_item` crashing on an empty `[hotkeys]` (it takes
  `childGroups().last()`) in place.
- The daemon and `foresthotkeys` share `services-app`: wire a
  `Displays::hotkeysChanged` signal to `reloadhotkeys` in `services.cpp`
  rather than a D-Bus call to itself. Deleting entries doesn't renumber:
  `foresthotkeys` ignores ids and `add_item` copes with gaps.
- No conflict warning for now. Only the Hotkeys page writes key sequences,
  and it has never had one.

### Confirm/revert, owned by the daemon

- Applying an **edited** layout that changes enable/mode/scale/transform
  shows a confirmation card on every enabled output: "Keep these display
  settings? Reverting in 15 s" [Revert] [Keep]. The daemon draws it (layer
  shell, overlay layer, `KeyboardInteractivityExclusive` on the primary
  screen's card; Escape = revert, Enter = keep). It doesn't depend on the
  settings app's window, which may be on a screen that just went dark.
- Revert happens on timeout, button or Escape. A daemon restart mid-confirm
  is weaker now: Biome has already persisted the unconfirmed layout, so only
  auto-pick (when a profile matches) restores a known-good layout. This is a
  phase 4 edge case.
- No confirmation for position-only or primary-only edits, or for applying
  a saved profile unedited (hotkey, Apply on a selected profile, auto-pick).
- A primary-only edit skips the modeset: the daemon writes the primary
  without calling `apply` when the layout already matches the live state.

### Primary screen

- Part of the layout and the profile. On every apply (saved or not) the
  daemon writes `display/primary_screen` in `Forest.conf`. That's the value
  `ScreenTracker::primary()` already reads, so consumers don't change.
- The layout JSON (`applyLayout`, `saveProfile*`) becomes
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
  mismatch), desktop → `handleScreenChange`. The lock screen and logout
  don't follow live. The lock card follows keyboard focus rather than the
  primary, and the primary can't change while locked. Logout is a
  short-lived dialog that picks the primary when it opens.

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
active=3f2c…            ; profile equal to the live layout, empty if none

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
| `applyLayout(QString json)` | apply a layout + primary with confirm/revert; saves nothing. After Keep, the matching profile equal to it (if any) becomes active |
| `saveProfile(QString id, QString json)` | overwrite a profile's layout + primary; doesn't apply |
| `saveProfileAs(QString name, QString json) → id` | save as a new profile; doesn't apply |
| `renameProfile(id, name)`, `deleteProfile(id)` | also update/remove the profile's hotkeys (3b); deleting the active one clears `active` |
| `identify()` | show the identify overlay |
| `profilesChanged`, `activeProfileChanged(id)`, `primaryChanged(name)` | signals for the editor and `ScreenTracker` |
| `keepLayout()`, `revertLayout()` | answer a pending confirmation (the card calls the same code) |
| `confirmPending`, `layoutKept`, `layoutReverted`, `applyFailed` | confirm-card lifecycle, for the editor |

## UI: System settings → Displays

The page edits a **working layout** loaded from the combo's selection.
Selecting something never applies it. The top of the page is one control
group: Profile, Primary display, Status, then the arrangement box with the
buttons under it. The per-output controls are a second group below.

- **Profile combo.** Unsaved entries come first,
  in italics: "Current layout" when the live layout equals no saved profile,
  and "Unsaved changes" while there are edits. Then every saved profile,
  with the ones matching the connected monitors first and a separator
  before the rest. On open (and on outside changes while there are no
  edits), the page selects the active profile, or "Current layout".
  Switching away from unsaved edits asks "Discard changes?" first.
- **Loading a profile:** `resolve()` maps it onto the live heads.
  - **View-only** if any of its outputs isn't connected. The canvas shows
    the missing outputs as red-tinted, dashed "Not connected" tiles, and every control
    except Rename and Delete is disabled. Every editable output is
    therefore a live head with a mode list, and `test` always works.
  - Connected monitors the profile leaves out are added as disabled outputs
    (in the disabled strip, so they can be enabled). That counts as an edit
    straight away ("Changed from Desk: HDMI-A-1 added").
- **Status row**, showing what's selected and what Apply would do. The
  state word is colour-coded through a `status` QSS property:

  | Selection | Status | Enabled |
  |---|---|---|
  | active profile, no edits | **Active** | Save, Rename, Delete |
  | matching profile, not live | **Not applied** | Apply, Save, Rename, Delete |
  | "Current layout" | **Applied, not saved** | Save |
  | edits | **Changed from <name>, not applied** (or **Changed, not applied**) | Apply, Save, Revert |
  | profile with a disconnected output | **View only: HDMI-A-1 isn't connected** | Rename, Delete |

  Apply is disabled rather than failing. The status row gives the reason.
- **Apply:** a matching profile with no edits → `applyProfile` (no
  confirm). Anything else → `applyLayout` (confirm when `needsConfirm`).
  Neither saves.
- **Save…:** a dialog with "Replace: [profile ▾]" (preselected to the
  profile the edits started from) or "New profile: [name]" (default
  `defaultName`). It calls `saveProfile` / `saveProfileAs` and doesn't apply.
  It needs a passing `test` when there are edits.
- **Rename** (inline: the combo swaps for a line edit; Enter commits, Escape
  cancels), **Delete** (asks first). Both act on the selected saved profile.
- **Primary display:** a combo box of the working layout's enabled outputs,
  page-level rather than a per-output checkbox (which couldn't be
  unchecked). Changing it is an edit, but needs no `test`.
- **Arrangement canvas** (`QWidget` with one `#DisplaysOutput` button per
  enabled output): sized by effective size and scaled to fit, labelled with
  name + model. Drag to move. Snap to the edges/centres of the others. On
  release, normalize the top-left to (0,0) and enforce connectivity. Disabled
  outputs sit in a strip below the canvas, in the same box, as dashed tiles.
  Click to select.
- **Selected output:** Enabled, Resolution, Refresh rate, Scale (presets
  100–300 % in 25 % steps plus custom), Orientation (Normal / 90 / 180 / 270,
  plus Flipped variants).
- **Buttons:** Identify (connector + model on each connected screen for ~3 s:
  a pass-through overlay card from the daemon, next to `ConfirmCard`), then
  Revert (discard edits), Save…, Apply.
- The page refreshes from the protocol on `done` and from daemon signals.
  Without edits, it follows a hotkey switch straight away. With edits, it
  keeps them and only updates the status row.
- Styling via `settings.css` object names, like the other system-settings
  pages.

## Phases

Each phase ends in something the user can manually test (keyboard/visual
testing is the user's, per the usual workflow).

Phase 1 (backend, no UI: `library/outputs`, the `displays` service,
`DBUS:` `arg=`, Biome persisting applies), phase 2 (Displays page,
confirm/revert apply), phase 3a (Apply/Save split, profile editing,
primary display, identify) and phase 3b (profile hotkeys on the Hotkeys
page, `library/hotkeyconfig`) are done.

### Phase 4 — hardening and wrap-up

- [ ] Edge cases: identical monitors without serials, an output unplugged
      mid-confirm, a daemon restart mid-confirm with no matching profile
      (Biome has already persisted the unconfirmed layout), saved mode no
      longer offered, all outputs disabled (refuse), mirroring (overlap)
      shown sensibly on the canvas.
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
