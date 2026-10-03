# Native popup menus → QMenu migration

## Why

The tray's context menu comes from `dbusmenu-lxqt`'s `DBusMenuImporter`,
which always builds a real `QMenu` (no way to make it build Forest's
`popupmenu` instead). Converting Forest's own popup menus to `QMenu` +
QSS gives the tray menu matching styling for free; until then it ships
unstyled on purpose (no one-off tray QSS).

## Current native popup-menu system

- `panel/panel-library/popupmenu.h` — `popupmenu`, `pmenuitem` (a styled
  `QPushButton`), `menuseperator`. Flat only: windowlist's "Move to desktop"
  (`windowbutton.cpp`) fakes a submenu by opening a second `popupmenu`.
- `panel/panel-library/popup.h` — the positioned window. Uses `Qt::ToolTip`
  rather than `Qt::Popup` because an `xdg_popup` grab needs a recent input
  serial, which hotkey/D-Bus-triggered opens don't have; click-outside is
  hand-rolled instead.
- Consumers (grep `new popupmenu(` / `popupmenu *` under `panel/panel-plugins/`):
  `mainmenu`, `windowlist`, `volume`, `clock`, `sensors`, `memorymonitor`,
  `cpumonitor`, `quicklaunch`, `nmcontrol`, `deskswitch`.

## Task: convert `popupmenu`/`pmenuitem`/`menuseperator` → `QMenu`/`QAction`

- Replace each consumer with `QMenu` + `QAction`s.
- Write QSS for `QMenu`, `QMenu::item`, `QMenu::separator` (+ hover/selected)
  in the theme CSS to match the current `#popup` / `#popupMenuItem` /
  `#popupMenuSeperator` rules in `usr/share/forest/themes/base/forest.css`.
- Decide what replaces the `Qt::ToolTip` workaround: `QMenu` always grabs.
  Check per consumer whether it can open without preceding input (mainmenu
  can — it has a hotkey); those that can't don't need the workaround.
- Use `QMenu::addMenu()` for windowbutton's desktop submenu.
- Autohide: `AutoHideManager::is_panel_popup()` only sees windows whose
  `QWindow::transientParent()` chains to the panel shell. Give each `QMenu` an
  explicit transient parent, or the panel may collapse under it — already the
  case for mainmenu's parentless `new QMenu` in `contextmenu.cpp`.

## Known issue (upstream, revisit during the task)

nm-applet's "VPN Connections" submenu is positioned wrong vertically. Biome
passes its `xdg_positioner` through unmodified; the anchor rect itself is
outside the parent menu's geometry, pointing at `dbusmenu-lxqt`/Qt's Wayland
submenu positioning.
