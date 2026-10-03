# Native popup menus → QMenu migration

## Why

- The tray's context menu comes from `dbusmenu-lxqt`'s `DBusMenuImporter`,
  which always builds a real `QMenu`. Converting Forest's own menus to
  `QMenu` + QSS styles the tray menu for free. Until then it ships unstyled
  on purpose (no one-off tray QSS).
- Real submenus: windowlist's "Move to desktop" fakes one today, and the
  planned directory-menu plugin (`roadmap.md`) needs nested cascades.
- Keyboard navigation, disabled/checkable items, and grab-based dismissal
  all come for free.

## Current state

- `panel/panel-library/popupmenu.h`: `popupmenu`, `pmenuitem` (a styled
  `QPushButton`), `menuseperator`. Flat only.
- `panel/panel-library/popup.h`: the positioned window, also used directly
  by non-menu popups (mainmenu launcher, volume, clock calendar, battery,
  sensors, nmcontrol, windowlist thumbnails). Those **stay on `popup`**.
  Only `popupmenu` goes away.
- `popupmenu` consumers: `mainmenu`, `windowlist` (+ `windowbutton`),
  `volume`, `clock`, `sensors`, `memorymonitor`, `cpumonitor`,
  `quicklaunch`, `nmcontrol`, `deskswitch`. `panel-app/panel.cpp` hands
  every plugin a "Panel Settings" `pmenuitem` through
  `panelpluginterface::setupPlug(QBoxLayout*, QList<pmenuitem*>)`.
- Already `QMenu` (all in the `forest` process, so they share the app
  stylesheet): tray menus (`systray/trayicon.cpp`), mainmenu's app
  right-click (`mainmenu/contextmenu.cpp`), and the desktop and desktop-icon
  menus (`desktop-app/desktop.cpp`, which has a "Create New" submenu).
- No theme has any `QMenu` rules yet.
- `#popupMenuItem` is also the object name of mainmenu's launcher list
  entries (`mainmenu/menuitem.cpp`, which also reads its icon size from
  that rule), so that rule can't simply be deleted.

## Findings

### Biome: nothing blocks this

- **Grab serials.** QtWayland (6.8.2) makes a `Qt::Popup` an `xdg_popup` and
  grabs it with the last *press* serial (button or key). It only falls back
  to a toplevel if the process has had no input at all
  (`QWaylandXdgSurface` ctor). Panel menus open on button release
  (`panelbutton::mouseReleaseEvent`), right after a press, so the grab
  serial is valid per spec on any compositor. wlroots 0.18 ignores the
  serial anyway. The `Qt::ToolTip` workaround in `popup.h` exists for
  hotkey-opened popups (the main-menu launcher), not for menus.
- **Constraints.** Biome already calls `wlr_xdg_popup_unconstrain_from_box`
  for layer-shell popups, popup-on-popup chains, and `reposition`. wlroots
  implements slide/flip/resize itself, so the positioner values below work
  without changes.
- **Keyboard and dismissal.** Grabbing popups get keys and outside-click
  dismissal through wlroots' popup grab. Clicks on Forest's own surfaces
  reach Qt, which closes the menu.
- **Not changing here:** `popup_wants_keyboard_focus()`
  (`biome/desktop/xdg_shell.cpp`) gives focus to every layer-shell-owned
  popup chain. That's a Forest-shaped exception that only `popup.h`'s
  non-grabbing popups need. QMenus grab, so the standard grab branch covers
  them. Don't add anything that leans on the layer-shell branch: on Biome
  both branches say yes, so testing can't catch it. Rethinking `popup.h` is
  tracked in both roadmaps.

### Positioning: why it breaks today, and the fix

- **Root cause.** LayerShellQt (6.3.4) never tells Qt where a layer surface
  is, so Qt puts the panel and wallpaper at (0,0) in its global frame.
  `QMenu` 6.8.2 computes popup positions and clamps them to
  `QScreen::availableGeometry()` in that bogus frame:
  - A root menu's position only works by accident: QtWayland's default
    anchor is `menu pos − parent pos`, which is still in the parent's own
    frame.
  - A submenu gets clamped against a screen rect that doesn't line up with
    its parent. The anchor rect then lands outside the parent menu, which is
    the nm-applet "VPN Connections" bug. It isn't `dbusmenu-lxqt`'s or
    Biome's fault. The desktop's "Create New" submenu has the same bug near
    screen edges.
- **Fix: set the xdg_positioner properties ourselves.** QtWayland reads the
  `_q_waylandPopupAnchorRect` / `Anchor` / `Gravity` / `ConstraintAdjustment`
  dynamic properties every time it creates a popup's shell surface. That
  happens on every show, and it's the same mechanism `popup.h` already uses.
  When these properties are set, QMenu's own (wrong) position is ignored for
  placement. Qt then takes the compositor's configured position back into
  its geometry, so later submenu math stays consistent.
- **Timing.** The properties have to be on `menu->windowHandle()` before the
  platform window is shown. `QShowEvent` is sent before `show_sys()`, so an
  app-wide `QEvent::Show` filter is early enough.
- **Upstream.** Qt 6.11 makes `QMenu` Wayland-aware
  (`QWaylandWindow::setParentControlGeometry` + Menu/SubMenu window types).
  For submenus it uses anchor `TopRight`, gravity `BottomRight`, and
  `flip_x | slide_y`, which Forest should match. The dynamic properties
  still take precedence in 6.11, so Forest's version stays correct after a
  Qt upgrade. Root menus opened at a point still need Forest's anchors even
  in 6.11, and so do submenus (see Phase 1's margin note).
- **Multi-monitor caveat.** `QMenu` picks `screenAt(p)` for size limits,
  and `p` is in the bogus frame, so very tall menus on a smaller secondary
  screen may size against the wrong screen. Placement is still correct.
  Acceptable for now.

### Styling: QSS can do what the current menus do

Checked against `qstylesheetstyle.cpp` 6.8.2:

| Current rule | QMenu equivalent | Notes |
|---|---|---|
| `#popup` background / border / radius / padding / margin | `QMenu { … }` | `padding` → `PM_MenuH/VMargin`. `margin` stays transparent. |
| rounded corners | `QMenu { border-radius }` + `WA_TranslucentBackground` | Needs the attribute *before* the native window exists (see below). |
| `#popupMenuItem` height / padding / border / radius / font | `QMenu::item { min-height; max-height; padding; border…; border-radius }`, `QMenu { font-size }` | Any box or border on `::item` switches Qt to full stylesheet drawing, which is what we want. `font-size` on `::item` makes Qt size icon items too narrow. Icons ignore `::item` padding, so they're placed with `QMenu::icon { left }`. |
| `icon-size: 22px` on the item | `QMenu { icon-size: 22px }` | `PM_SmallIconSize` reads it from the **QMenu** rule, not `::item`. |
| `:hover` | `QMenu::item:selected` | Also the keyboard highlight. |
| `:pressed` | `QMenu::item:selected:pressed` | QMenu sets `State_Sunken` while the mouse is down. |
| `#popupMenuSeperator` | `QMenu::separator { height: 1px; margin: 4px 2px; background }` | Margins add to the row height. |
| (none today) | `QMenu::right-arrow`, `QMenu::indicator:checked/:exclusive`, `QMenu::item:disabled` | Needed for tray menus and submenus. |

- **Translucency hook.** An app-wide `QEvent::Polish` filter sets
  `WA_TranslucentBackground` on every `QMenu`. `QMenu::popup()`/`exec()`
  polish before creating the window. That catches menus Forest doesn't
  construct itself: `DBusMenuImporter`'s menus and submenus, and Qt's own
  `QLineEdit` context menu in the mainmenu search box. Anything that calls
  `winId()` on a menu early must call `ensurePolished()` first.
- **Fallback** if QSS can't express something: a `QProxyStyle` under the
  stylesheet style, which is still app-wide and still covers dbusmenu menus.
  Only use it if the spike proves it's needed.
- No shadows or animations wanted. QMenu's fade/scroll effects
  (`UI_AnimateMenu`/`UI_FadeMenu`) create extra windows, so disable them if
  the platform theme turns them on.

## Plan

### Phase 0: styling spike (no C++ consumer changes) — done

1. Add `QMenu` / `QMenu::item` / `::separator` / `::right-arrow` /
   `::indicator` rules next to the existing `#popup*` rules in `base`,
   `base-light`, `base-dark`, `base-rounded`, and `base-circle` `forest.css`.
2. Add the `Polish` → `WA_TranslucentBackground` filter in the `forest`
   main process (`forest/forest.cpp`).
3. Compare side by side in all four visible themes. Already-`QMenu`
   testbeds, no conversions needed:
   - desktop right-click: icons, separators, a submenu
   - nm-applet tray menu: checkable items, disabled items, a submenu
   - mainmenu app right-click

   Compare against a current `popupmenu` (e.g. clock right-click).
4. Exit when the look is signed off. Positioning will still be off here;
   that's Phase 1.

### Phase 1: positioning helper — done

- `library/menuanchor`: `anchorMenu()`, `anchorMenuAtPoint()`, and the
  app-wide `MenuFilter` (translucency + submenu anchoring), installed in
  `forest.cpp`.
- `panel-library/panelanchor`: `PositionpPolicy` math shared by `popup` and
  `anchorMenuOnLauncher()`, the entry point for Phase 2's panel menus.
- Testbeds converted: tray (anchored on the `trayicon`), desktop and
  desktop-icon menus (`anchorMenuAtPoint`).
- **Margins.** The QSS `margin` is transparent window area, so anchors must
  offset by it or menus land beside the visible frame. `menuanchor` derives it
  as `PM_MenuPanelWidth − PM_DefaultFrameWidth`, which only holds while the
  `QMenu` rule declares a border (base sets `border: none`). Submenus anchor
  to the parent's visible frame and align first items, so keep the filter
  even after Qt ≥ 6.11: Qt's own submenu anchor ignores the margin.

### Phase 2: convert consumers — implemented, needs manual testing

1. Change `panelpluginterface::setupPlug` to `QList<QAction*>`, and make
   `panel.cpp`'s "Panel Settings" a `QAction`. All plugins are in-tree.
2. Convert each `popupmenu` consumer. windowbutton's desktop list becomes
   `addMenu()`.
3. Launcher toggle: `panelbutton` opens on release, so right-clicking the
   launcher while its menu is open lets the press close the menu and the
   release reopen it. Guard this in the panel helper (`popup.h` solves the
   same problem with `lastPressOnLauncher`).
4. `mainmenu/contextmenu.cpp`: anchor on the app entry. Its transient parent
   is the launcher popup window, which chains to the panel, so autohide
   still works.
5. Delete `popupmenu` / `pmenuitem` / `menuseperator` and the
   `#popupMenuSeperator` rules. Rename mainmenu's remaining
   `#popupMenuItem` to `#panelMainMenuItem` in every theme, including
   the `get_iconsize_stylesheet` lookup.

### Phase 3: cleanup

- Update `CLAUDE.md`'s `setupPlug` signature.
- Mark the directory-menu plugin as unblocked in `roadmap.md`.
- Delete this file.

## Test checklist (manual)

- Every converted menu in all four themes. Hover, keyboard
  (arrows/Enter/Esc), disabled and checkable items.
- Top and bottom panel positions. Launchers near both screen corners
  (slide). A submenu near the right edge (flip_x) and bottom (slide_y).
- Autohide panel: it stays revealed while a menu or submenu is open, and
  collapses after the menu closes.
- Click outside: on another app's window, on the desktop, on a different
  panel launcher, on the same launcher (toggle).
- Theme switch while running re-styles open and future menus.
