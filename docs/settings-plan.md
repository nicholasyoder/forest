# Settings App Rework — Plan

Turn `forest-settings` into a proper settings center: user-facing
categories instead of one top-level entry per plugin, tree navigation,
search (#25), deep links, and panel applet settings moved in from their
popup dialogs.

## Problems with the current design

- **Layout mirrors the code.** Each settings plugin returns its own
  top-level tree, so the home list is effectively one entry per plugin
  ("Services" = whatever `services-app` hosts). A plugin can't put a page
  into a category another plugin owns.
- **No stable identity.** Items get random `QUuid`s; deep links
  (`forest-settings Panel` from `panel.cpp` / `desktop.cpp`) match the
  display name, top level only.
- **Fragile ordering.** Top level is sorted through a `QMap` keyed by
  name (a duplicate name silently drops a plugin; About is first only
  alphabetically). `has_child_cats` looks only at the first child.
- **Two-level drill-down.** The sidebar is replaced by subcategories plus
  a Back row; the breadcrumb is fixed at two levels.
- **`description` doubles as search keywords** and nothing reads it.
- **Plugin discovery rides on the app-plugin list** in the user's
  `Forest.conf [plugins]` (`settings-only=true` entries), so every new
  settings plugin needs a `SettingsUpgradeManager` migration (see
  `upgrade_0_9_0`, which added `locker`).
- **Applet settings live outside the app**: old popup `settingswidget`s
  opened from applet context menus.
- **No single instance.** Each launch opens a new window.

## Categories

Owned by the settings app (fixed order, stable IDs, title, icon). Plugins
only say which category a page belongs in.

| ID | Title | Pages (now → later) |
|---|---|---|
| `about` | About | About |
| `appearance` | Appearance | Theme, Wallpaper, Cursor → Icon theme |
| `desktop` | Panel (→ Desktop & Panel) | Panel (+ one subpage per applet) → Desktop icons |
| `displays` | Displays | Displays |
| `input` | Input & Hotkeys | Hotkeys → keyboard / pointer / touchpad |
| `notifications` | Notifications | Notifications |
| `power` | Power & Lock | Lock Screen → battery, lid |
| `session` | Session & Startup | Autostart |
| `system` | System | → default apps, date & time |

Empty categories are hidden (`system` until its first page exists). Pages
whose category is unknown land in an `other` category at the end.

- About is a single-page category at the top of the tree, so it's both the
  first page and the default one (see Navigation).

- Panel → Behavior and Panel → Applets merge into one Panel page (behavior
  group + applet list), so applet pages sit one level down
  (`desktop/panel/clock`) instead of two.
- Multiple panels (#53) is out of scope, but the tree shape shouldn't
  depend on panel count: it would add a panel picker at the top of the
  Panel page (as Displays picks a monitor), not one tree entry per panel.
  Deep links stay `desktop/panel/...` plus a panel argument.
- Session → General (one checkbox, "Launch XDG autostart apps") folds
  into the Autostart page.
- Empty placeholders (Themes → Widget, Themes → Icon, the Desktop → Icons
  test button) are dropped; their roadmap items stay. The unused
  `desktop-settings/settingswidget.{h,cpp,ui}` goes too.
- Lock Screen's idle/display-off group may later split into its own
  "Screen blanking" page under `power`.
- Volume's dialog is device management (master sink, autosave), not applet
  appearance. It moves to `desktop/panel/volume` for now; a future `sound`
  category is the natural home once there's more to put there.

## Plugin interface

Replace `settings_plugin_infterace` (typo included) with a versioned
interface that returns a flat list of pages, not a tree:

```cpp
class settings_page : public settings_category {
    QString path;         // "desktop/panel/clock": deep link + placement
    int order = 0;        // position among siblings, then title
    QStringList keywords;
};

class settings_plugin_interface {
    virtual QList<settings_page*> pages() = 0;
};
Q_DECLARE_INTERFACE(settings_plugin_interface, "forest.settings.plugin.interface/2")
```

- The path is the only placement data: the first segment is the category,
  everything before the last segment is the parent page. Separate
  `category` / `parent` fields would just be redundant copies that can
  disagree. Moving a page changes its deep link, which is fine while every
  caller is in-tree.
- A page whose parent path doesn't exist is logged and attached at the top
  of its category, so load order between plugins never matters.
- Page contents keep today's model (`settings_widget`,
  `settings_widget_group`, custom widgets) and the `opened` / `updated`
  signals. Pages may have both contents and child pages (Panel).
- `description` becomes a real description only (shown as a subtitle);
  search terms move to `keywords`.
- `settings_item` keeps its `QUuid` for internal lookups; paths are for
  placement, search and deep links.

## Plugin loading

Settings plugins install to `/usr/lib/forest/settings/` and the app loads
every `.so` there. Installed means shown; no config entry needed.

- Drop the `settings-only` entries from `etc/forest/Forest.conf`, and the
  `SETTINGS_PLUGIN` / `settings-only` handling in
  `pluginutills::get_plugin_paths` (it becomes app-plugins only). A
  `SettingsUpgradeManager` step removes the entries from existing user
  configs.
- App plugins that also ship settings (`desktop`, `panel`, `services`)
  keep their `[plugins]` entry for the app side only. No `requires =
  "desktop"` hiding: nobody disables those app plugins in practice, and it
  can be added later without an interface change.
- Old `lib*-settings.so` files left in `/usr/lib/forest/` are harmless
  (nothing loads them any more); the `.deb` removes them, staging installs
  need a manual `rm`.

## Navigation

Replace `catlistwidget` + `breadcrumbwidget` with a QSS-styled
`QTreeView` over a `QStandardItemModel` (one item per category and page,
page pointer in a user role):

- Top level: categories. Clicking one expands it and opens its first page;
  a single-page category (Displays) opens directly with no children shown.
- Pages are leaves unless they have child pages (Panel → Clock).
- Expanding one category collapses the others (accordion), so the tree
  stays short.
- No home screen: with no argument the window opens the first page,
  About, with every category collapsed.
- The page stack (`stack_hash` / `QStackedLayout`) stays as is, keyed by
  page.

## Search (#25)

- Search field above the tree.
- Each page's tree item gets a hidden search-text role, built at load:
  page title, category title, keywords, and row labels
  (`settings_widget::name()`). Pages that build their rows on open
  (Displays, Hotkeys, Autostart) are searchable only through their title
  and keywords, so give them good keywords.
- Filtering: `QSortFilterProxyModel` on that role with
  `recursiveFilteringEnabled`, every query word must match; matching pages
  stay visible with their categories expanded. Empty query restores the
  accordion state.
- Opening a result that matched on a row label scrolls to that row
  (`QScrollArea::ensureWidgetVisible`) and briefly highlights it (a dynamic
  property styled in `settings.css`). Needs a row label → row frame map,
  filled in `display_widgets` and rebuilt on `updated`.

## Deep links and single instance

- `forest-settings <path>`, e.g. `forest-settings desktop/panel`. A
  category ID opens that category's first page; an unknown path logs and
  opens the first page.
- Single instance: the first instance owns `org.forest.Settings` on the
  session bus with `OpenPage(path, activation_token)`. Later launches call
  it and exit. Forest-internal, so the decoupling goal doesn't call for a
  standard interface here.
- Raising the existing window uses `xdg-activation-v1` (implemented in
  Biome; policy in its `docs/architecture-notes.md`). The launch forwards its `XDG_ACTIVATION_TOKEN` in
  `OpenPage`; QtWayland's `requestActivate()` reads the token from the
  environment (`qwaylandxdgshell.cpp`), so the receiver `qputenv`s it first
  (what KWindowSystem does).
- Launchers have to create that token. Qt 6.8 only exposes token requests
  as private API (`qwaylandwindow_p.h`), so add a small raw-protocol helper
  to `library/` (as `toplevels` does for its protocols) and use it in
  `panel.cpp`, `desktop.cpp` and the applet "Settings" actions. Biome only
  honors tokens with a seat + serial, minted while the clicked surface is
  still focused, so request it in the click handler before the menu hides.
  Without a token (terminal launch) the window is only marked urgent.
  `mainmenu`
  and `quicklaunch` can adopt it later so every app launched from the panel
  gets activation.

## Panel applet settings

Applets with settings ship a separate settings plugin next to their
panel plugin, e.g. `libclock-settings.so` in `/usr/lib/forest/settings/`,
using the same `settings_plugin_interface` and contributing a
`desktop/panel/<applet>` page. No second interface and no
`panel-settings` wiring.

- Why not have `panel-settings` `qobject_cast` the applet `.so` to an
  optional settings interface: the applet's root object is its panel
  widget, and some construct real state up front (`windowlist` builds its
  dialog as a member initializer). Loading applets in the settings process
  only to ask for pages invites side effects; a separate module has none,
  and follows the existing `-app` / `-settings` split.
- Shared config keys live in a small `<applet>config.h` (as
  `locker/lockerconfig.h` does for the locker) compiled into both sides.
- Pages that need runtime data (sensors' chip list, volume's sinks) query
  it themselves (libsensors, the audio engine sources) rather than asking
  the running panel.
- After a change the page calls `forest/panel/reloadappletsettings`, which
  calls a new non-pure `virtual void reloadSettings() {}` on every loaded
  applet. Adding a virtual changes the vtable, so bump the panel IID to
  `forest.panel.plugin.interface/3` (all applets are in-tree). The
  in-process `settingschanged` signals go away.
- Remove the popup dialogs: `settingswidget` in clock, cpumonitor,
  memorymonitor, volume and windowlist, plus sensors' `WidgetSensorConf`.
  Each applet's context-menu "Settings" action becomes
  `forest-settings desktop/panel/<applet>`.
- Every installed applet with settings gets a page, on the panel or not
  (falls out of "installed means shown"; lets one be set up before it's
  added). Multiple panels (#53) turns this into "which panel's clock" and
  is the time to move to per-instance settings files; the current ones
  (`"CPU Monitor"`, `"Window List"`, `"Temperature Monitor"`, …) stay.
- The Applets list on the Panel page links each row with settings to its
  page (row button or double-click).
- Redesign each page's contents while porting it, not as a 1:1 copy of
  the old dialog. cpumonitor and memorymonitor share their color/opacity
  rows; put a helper for those in `panel-library`.

## Theming

`base/settings.css` needs rules for the tree view, the search field and
the search-match highlight, and loses the `CategoryButton` / `BreadCrumb*`
rules. Per-theme `settings.css` overrides and theme-change reload (#56)
are independent of this plan, but easier after Phase 1 so the variant
files are written against the final widget set.

## Phases

Each phase is one PR into `develop` unless noted.

### Phase 1 — interface, loading, categories, tree, deep links

Done (`af2c807`..`b7918fd` on `develop`).

### Phase 2 — search

Done.

### Phase 3 — single instance

`org.forest.Settings` + `OpenPage`, forwarding launches with their
activation token, and the token helper in `library/` for the launchers.
Test that a panel/desktop-menu relaunch raises the window and a terminal
relaunch only marks it urgent.

### Phase 4 — applet settings infrastructure + clock

`reloadSettings()` on `panelpluginterface` (`/3`),
`reloadappletsettings` D-Bus slot, CMake pattern for `<applet>-settings`
modules, Applets-list links, and clock (the simplest dialog) as the first
port to prove the pattern.

### Phase 5 — remaining applets

cpumonitor + memorymonitor (shared color helper), windowlist, sensors,
volume. One PR, or one per applet if pages get redesigned substantially.
