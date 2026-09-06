# Native popup menus → QMenu migration

## Context

Landed 2026-09-06: the system tray's StatusNotifierItem port
(`panel/panel-plugins/systray/`, `services/services-app/systemtray/`) — see
`WAYLAND-TESTING-NOTES.md` for that port's own scaffolding notes. Its
right-click context menu comes from `dbusmenu-lxqt`'s `DBusMenuImporter`,
which builds and owns a real `QMenu`/`QAction` tree internally — there's no
way to redirect it to build Forest's own `popupmenu`/`pmenuitem` widgets
instead (checked: `DBusMenuImporter::createMenu()` still returns a `QMenu*`).

Two things were deliberately deferred out of that work, decided with the
user while reviewing the plan and again after manual testing:

1. Convert Forest's *native* popup-menu system to plain `QMenu`/`QAction`
   too, with QSS written to make `QMenu` look like Forest's current custom
   popups. Once that lands, the tray's `dbusmenu-lxqt`-provided `QMenu`
   picks up the same styling for free — no tray-specific work needed. This
   is why the tray's own menu currently ships completely unstyled rather
   than getting one-off QSS.
2. A real bug found in manual testing: the tray's `QMenu` opens **downward
   off-screen** when right-clicked, because Forest's panel is bottom-docked
   by default and the menu doesn't flip upward to fit. Deferred because it's
   architecturally the same problem `panel/panel-library/popup.h` was built
   to dodge (see below) — worth solving once, alongside item 1, not as a
   tray-only patch.

This file tracks both for whoever picks this up next.

## Current native popup-menu system

- `panel/panel-library/popupmenu.h` — `popupmenu` (container), `pmenuitem`
  (a `QPushButton` styled via `objectName("popupMenuItem")`),
  `menuseperator`. Flat only — no native submenu support. Windowlist's
  "Move to Desktop" item (`windowbutton.cpp`) fakes a submenu today by
  opening a *second* `popupmenu` on click; a real `QMenu` has native
  `addMenu()` submenu support, so that workaround likely goes away/simplifies
  as part of this conversion, not just gets ported as-is.
- `panel/panel-library/popup.h` — the actual positioned/shown window
  (`popup` class), taking a `PositionpPolicy` (`CenteredOnWidget` /
  `EdgeAlignedOnWidget` / `CenteredOnMouse` / `EdgeAlignedOnMouse`). Its own
  header comment documents *why* it deliberately uses `Qt::ToolTip` instead
  of a real popup grab (`Qt::Popup`): a real Wayland `xdg_popup` grab needs a
  valid, recent input serial, which a hotkey/D-Bus-triggered popup never
  has — a currently-unresolved-upstream wlroots/KWin limitation, not
  something specific to Forest or Biome. `Qt::ToolTip` sidesteps it
  entirely (no WM management, no grab needed) at the cost of hand-rolling
  click-outside-closes-it via an app-wide event filter, which `popup.h`
  already does.
- Current real consumers (construct/hold a `popupmenu` directly — grep
  `new popupmenu(` and `popupmenu *` under `panel/panel-plugins/`):
  `mainmenu`, `windowlist` (`windowbutton.cpp`, including the submenu
  chaining trick above), `volume`, `clock`, `sensors`, `memorymonitor`,
  `cpumonitor`, `quicklaunch`, `nmcontrol`, `deskswitch`. That's effectively
  every panel plugin with a click-to-open popup — a real, multi-file
  conversion, not a small one.

## Task 1: convert `popupmenu`/`pmenuitem`/`menuseperator` → `QMenu`/`QAction`

- Replace each of the consumers above with plain `QMenu` + `QAction`s.
- Author QSS for `QMenu`/`QMenu::item`/`QMenu::separator` (and selected/hover
  pseudo-states) in the theme CSS files (`usr/share/forest/themes/base/*.css`
  etc.) to visually match what `#popup`/`#popupMenuItem`/`#popupMenuSeperator`
  currently produce — check `usr/share/forest/themes/base/forest.css` for the
  current rules to match.
- Decide what replaces `popup.h`'s `Qt::ToolTip` grab workaround for a real
  `QMenu` (which always uses a genuine popup grab) — confirm whether
  hotkey/D-Bus-triggered menus among the consumers above actually exist
  today (if none of the current `popupmenu` consumers are ever opened
  without real preceding click/key input, the grab-serial gap noted in
  `popup.h` may simply not apply here and can be a non-issue — verify
  per-consumer rather than assuming).
- `windowbutton.cpp`'s desk-menu submenu chaining: re-evaluate using
  `QMenu::addMenu()` for a real submenu instead of the two-popup chain.

## Task 2: fix `QMenu` popup positioning under Wayland

**Symptom:** right-click the tray → `QMenu` opens downward from the cursor
and runs off the bottom of the screen, because Forest's panel is
bottom-docked and nothing flips the menu upward to compensate.

**Why this isn't a quick fix:** `QMenu::popup()` uses Qt's Wayland QPA
`xdg_popup` path — a real compositor-negotiated popup grab with an
`xdg_positioner` that's supposed to support constraint/flip adjustment so
the compositor repositions the popup to fit on screen. `popup.h`'s own
header comment (see above) already documents that this class of Wayland
popup/grab behavior has real, currently-unresolved gaps in the
wlroots/KWin ecosystem generally — `popup.h` sidesteps the *grab-serial*
half of that gap by not using a real popup at all. `QMenu` has no such
escape hatch, so this needs actual investigation, likely in two places:

- **Biome side:** check `desktop/xdg_shell.cpp` (or wherever Biome handles
  `xdg_popup`/`xdg_positioner`) for whether it implements the positioner's
  `constraint_adjustment` bits (specifically the flip-on-Y-axis case) at
  all. If not, this is a real compositor-side gap worth closing on its own
  merits (standard-protocol compliance, matches the project's decoupling
  goal — see `biome/docs/plan.md`'s "Decoupling goal" section — rather than
  a Forest-specific workaround).
- **Forest/Qt side:** even with a correct positioner, confirm Qt's own
  Wayland QPA is asking for the right anchor rect/gravity in the first
  place — worth checking what `QMenu::popup(pos)` actually requests
  relative to the launching widget's screen position and Forest's own
  panel-reserved exclusive zone (layer-shell), since a wrong *request* can't
  be fixed by a correct positioner on the compositor side.

**Why it blocks/overlaps Task 1, not just the tray:** none of today's
`popupmenu` consumers hit this bug only because they deliberately avoid a
real popup grab (`Qt::ToolTip`, per `popup.h`). The moment they're converted
to real `QMenu`s (Task 1), every one of them becomes newly exposed to this
same positioning bug — so this needs solving *before or alongside* Task 1
lands, not treated as a tray-only nicety afterward.
