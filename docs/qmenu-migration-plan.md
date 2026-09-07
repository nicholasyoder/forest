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

   **Resolved 2026-09-06 — fixed in Biome, not Forest.** See Task 2 below:
   root-caused via a `WAYLAND_DEBUG` trace to a Biome gap (a client's
   `xdg_popup.reposition()` request — which Qt sends right after creating
   the tray's popup, once its real content size is known — was never
   re-run through Biome's screen-constrain logic, only the popup's initial
   creation was). Fixed compositor-side in `desktop/xdg_shell.cpp`
   (`constrain_popup_to_output()` + a new `reposition` listener). Two
   Forest-side attempts at a client-side positioning workaround
   (`trayicon.cpp`) were tried first, had no effect (confirming the bug
   wasn't client-side), and were reverted once the real cause was found.
   Since this fix is compositor-level, it benefits every `QMenu`/`xdg_popup`
   under Biome, not just the tray's — Task 2 is effectively done, and Task 1
   is no longer blocked on it.

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

## Task 2: fix `QMenu` popup positioning under Wayland — RESOLVED 2026-09-06

**Symptom:** right-click the tray → `QMenu` opened downward from the cursor
and ran off the bottom of the screen (Forest's panel is bottom-docked,
nothing flipped the menu upward). Two client-side (Forest) fixes were tried
and had zero effect — expected in hindsight, since Wayland gives clients no
global-coordinate space to compute a "corrected" point in.

**Root cause and fix, both in Biome, not Forest:** a `WAYLAND_DEBUG=1` trace
showed Qt's `QMenu` positioner creates the popup with a placeholder size,
then calls `xdg_popup.reposition()` with the real size once content is
known (wlroots 0.18's `wlr_xdg_popup::events.reposition`). Biome's existing
screen-constrain logic (`wlr_xdg_popup_unconstrain_from_box()` in
`desktop/xdg_shell.cpp`) only ran at popup creation, not on reposition, so
the corrected — larger, more likely genuinely off-screen — geometry sailed
through unconstrained. Fixed by extracting the constrain logic into
`constrain_popup_to_output()` and adding a `reposition` listener that
re-runs it on every reposition, not just creation. A same-day follow-up bug
(nm-applet's Wi-Fi submenu landing in the wrong place) had a related cause:
the constrain box must be relative to the popup's *root* toplevel parent,
not its immediate parent, which only matters once a popup opens another
popup (a submenu) — fixed by walking the real `xdg_popup->parent` chain
instead of the scene tree to find the true root. Since this fix is
compositor-level, it benefits every `QMenu`/`xdg_popup` under Biome, not
just the tray's — Task 1 is not blocked on it.

**Known remaining issue, not fixed — likely Qt-side, revisit during Task 1:**
nm-applet's "VPN Connections" submenu is still positioned wrong vertically.
A trace showed Biome passing that popup's `xdg_positioner` through
completely unmodified — there's nothing to slide/flip because Biome never
judged it out-of-bounds. The anchor rect itself looks invalid per the
xdg-shell spec (x-coordinate outside the parent menu's own window
geometry), pointing at `dbusmenu-lxqt`/`QMenuPrivate`'s Wayland submenu
positioning sending a bad anchor rect — a real fix would live upstream (Qt
or dbusmenu-lxqt), not in Forest or Biome.
