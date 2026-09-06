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

**Symptom:** right-click the tray → `QMenu` opens downward from the cursor
and runs off the bottom of the screen, because Forest's panel is
bottom-docked and nothing flips the menu upward to compensate.

**Investigation:** two client-side (Forest) fixes were tried first and
both had zero observable effect:
1. Computing an already-on-screen `QPoint` (anchored above the tray icon,
   clamped to `screen()->availableGeometry()`) instead of passing
   `QCursor::pos()` to `menu()->popup()`.
2. Waiting for `DBusMenuImporter::menuUpdated()` before popping up, in case
   `sizeHint()` was being measured against a not-yet-populated menu.

Neither changed the symptom at all, which was itself the clue: Wayland
gives clients no real global desktop-coordinate space to compute a
"corrected" point in, so a client-side position fix can't be the answer.
Root-caused instead via a live `WAYLAND_DEBUG=1` trace of the actual
`xdg_positioner`/`xdg_popup` protocol traffic during the tray right-click:

- Qt's positioner for `QMenu` always requests `anchor=top_left`,
  `gravity=bottom_right`, `constraint_adjustment=slide_x|slide_y` (no flip
  bits, ever) — a hardcoded Qt/QtWaylandClient default, not something a
  Forest-side `QPoint` can influence.
- Biome (`desktop/xdg_shell.cpp`) *does* correctly call wlroots'
  `wlr_xdg_popup_unconstrain_from_box()` to slide/flip a popup back on
  screen — but only in response to `wlr_xdg_popup`'s creation (`new_popup`).
  The trace showed Qt creating the popup with a placeholder size, then
  immediately calling `xdg_popup.reposition()` with the real, final size
  once the menu's actual content was known (real wlroots 0.18 signal:
  `wlr_xdg_popup::events.reposition` — this didn't exist in wlroots 0.15,
  which is what `misc/wlroots`'s vendored reference checkout has, so don't
  trust that copy for this). Biome had no listener on that signal at all,
  so the corrected (and larger, more likely to be genuinely off-screen)
  geometry that actually got shown to the user sailed through completely
  unconstrained.

**Fix (Biome, not Forest):** `desktop/xdg_shell.cpp` — extracted the
existing constrain logic into `constrain_popup_to_output()` (generalized to
read the popup's own scene-node parent rather than assuming a specific
caller's parent-lookup shape, so it works for both real-`xdg_surface`-parent
popups and layer-shell-owned ones like the panel's tray menu) and added a
`BiomePopup::reposition` listener on `wlr_xdg_popup::events.reposition` that
re-runs it on every reposition, not just at creation. Both Forest-side
attempts above were reverted (`trayicon.cpp` is back to plain
`menu()->popup(QCursor::pos())`) since the real fix is compositor-level and
benefits every `QMenu`/`xdg_popup` under Biome, not just the tray's — Task 1
is no longer blocked on this.

**Follow-up fix, same day:** manual testing found a second, related bug —
nm-applet's Wi-Fi-list submenu (a popup-on-popup, opened from the tray's
already-open context menu) landed in the wrong place on both axes,
independent of the reposition fix above. Root cause: `wlr_xdg_popup_unconstrain_from_box()`'s
own doc comment requires its box to be relative to the popup's *root
toplevel parent surface*, not just its immediate parent — for a one-level
popup those are the same surface (so the original fix's use of
`scene_tree->node.parent` happened to work), but for a submenu the
immediate parent is another popup, not the root, and using its position
silently shifted every flip/slide computation by the gap between them.
Fixed by walking the real `xdg_popup->parent` chain (not the scene tree,
which looks identical at every level) to find the actual root popup before
resolving its scene-parent's absolute position — see the updated comment on
`constrain_popup_to_output()` for why `xdg_popup->parent` is the right link
to walk (it correctly stops at a layer-shell-owned root too, since that
attachment is invisible to it).

**Known remaining issue, not fixed — likely Qt-side, needs revisiting during
Task 1:** nm-applet's "VPN Connections" submenu (single item, opens off the
main tray menu) is still positioned wrong vertically — noticeably higher
than the actual on-screen row that triggers it. A `WAYLAND_DEBUG` trace of
this specific submenu showed Biome passing the popup's `xdg_positioner`
through **completely unmodified** (requested and final `configure` values
identical - `set_anchor_rect(-196, 260, 1, 1)` → `configure(-196, 260, 196,
22)`), so this isn't the same class of bug as above; there's nothing to
slide/flip because Biome never judged it out-of-bounds. The anchor rect
itself looks suspect: x=-196 is outside the parent menu's own window
geometry (parent width is only 262px), which the xdg-shell spec's own
`set_anchor_rect` doc says must not happen ("the anchor rectangle may not
extend outside the window geometry of the ... parent surface") - pointing at
`dbusmenu-lxqt`'s `QMenu` (specifically `QMenuPrivate`'s Wayland submenu
positioning) sending a bad/stale anchor rect, not a Biome bug. Not
independently confirmed against a known upstream Qt bug report yet - worth
a proper search once Task 1 work resumes, since a real fix would live
upstream (Qt or dbusmenu-lxqt), not in Forest or Biome, matching this file's
opening note that `DBusMenuImporter`'s `QMenu` can't be redirected to
Forest's own widgets at all.
