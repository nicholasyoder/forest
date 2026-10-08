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

Owned by the settings app (fixed order, stable IDs). Plugins only say
which category a page belongs in.

| ID | Title | Pages (now → later) |
|---|---|---|
| `appearance` | Appearance | Theme, Cursor → Icon theme |
| `desktop` | Desktop & Panel | Wallpaper, Panel, Applets (+ one subpage per applet) → Desktop icons |
| `displays` | Displays | Displays |
| `input` | Input & Hotkeys | Hotkeys → keyboard / pointer / touchpad |
| `notifications` | Notifications | Notifications |
| `power` | Power & Lock | Lock Screen → battery, lid |
| `session` | Session & Startup | Autostart |
| `system` | System | About → default apps, date & time |

Unknown category IDs land in an `other` category, shown only if non-empty.

- Session → General (one checkbox, "Launch XDG autostart apps") folds
  into the Autostart page.
- Empty placeholders (Themes → Widget, Themes → Icon, the Desktop → Icons
  test button) are dropped; their roadmap items stay.
- Lock Screen's idle/display-off group may later split into its own
  "Screen blanking" page under `power`.

## Plugin interface

Replace `settings_plugin_infterace` (typo included) with a versioned
interface that returns pages, not a tree:

```cpp
class settings_page : public settings_category {
    QString id;          // "desktop/panel/applets/clock"; also the deep-link path
    QString category;    // "desktop"
    QString parent;      // parent page id, for nested pages; empty = top of category
    int order;           // position among siblings
    QStringList keywords;
};

class settings_plugin_interface {
    virtual QList<settings_page*> pages() = 0;
};
Q_DECLARE_INTERFACE(settings_plugin_interface, "forest.settings.plugin.interface/2")
```

- Page contents keep today's model (`settings_widget`,
  `settings_widget_group`, custom widgets) and the `opened` / `updated`
  signals.
- `description` becomes a real description only (shown as a subtitle);
  search terms move to `keywords`.
- `settings_item` keeps its `QUuid` for internal lookups; IDs are for
  placement, search and deep links.

## Plugin loading

Settings plugins install to `/usr/lib/forest/settings/` and the app loads
every `.so` there. Installed means shown; no config entry needed.

- Drop the `settings-only` entries from `etc/forest/Forest.conf`, and the
  `settings-only` handling in `pluginutills::get_plugin_paths`. A
  `SettingsUpgradeManager` step removes them from existing user configs.
- App plugins that also ship settings (`desktop`, `panel`, `services`)
  keep their `[plugins]` entry for the app side only.
- Optional: a page may name an app plugin it requires
  (`requires = "desktop"`) and is hidden when that plugin is disabled.

## Navigation

Replace `catlistwidget` + `breadcrumbwidget` with a QSS-styled
`QTreeView` over a `QStandardItemModel`:

- Top level: categories. Clicking one expands it and opens its first page;
  a single-page category (Displays) opens directly with no children shown.
- Pages are leaves; nested pages (Panel → Applets → Clock) add levels.
- Expanding one category collapses the others (accordion), so the tree
  stays short.
- The page stack (`stack_hash` / `QStackedLayout`) stays as is, keyed by
  page.

## Search (#25)

- Search field above the tree.
- Index, built at load (all items already exist then): page title,
  keywords, and the row labels (`settings_widget::name()`) of generic
  pages. Custom-widget pages (Displays, Hotkeys, Autostart) are searchable
  only through their title and keywords, so give them good keywords.
- Filtering: `QSortFilterProxyModel` with `recursiveFilteringEnabled`;
  matching pages stay visible with their categories expanded.
- Opening a result that matched on a row label scrolls to that row and
  briefly highlights it (a dynamic property styled in `settings.css`).

## Deep links and single instance

- `forest-settings <page-id>`, e.g. `forest-settings desktop/panel`.
  Update the callers in `panel.cpp` and `desktop.cpp`. A category ID opens
  that category's first page.
- Single instance: the first instance owns `org.forest.Settings` on the
  session bus with an `OpenPage(id)` method. Later launches call it (which
  raises the window) and exit. This is Forest-internal, so the decoupling
  goal doesn't call for a standard interface here.

## Panel applet settings

Applets that have settings implement a second, optional interface next to
`panelpluginterface`, so applets without settings stay unchanged:

```cpp
class panel_applet_settings_interface {
    virtual QList<settings_item*> settings_items() = 0;  // page contents
};
```

- `panel-settings` already loads every panel plugin `.so` for the Applets
  list. It `qobject_cast`s each one to the new interface and adds a
  `desktop/panel/applets/<name>` page for those that implement it.
  Settings-side code must not depend on `setupPlug()` having run.
- After a change, the page calls a panel D-Bus slot that reloads that
  applet's settings (the in-process `settingschanged` signals go away).
- Remove the popup dialogs: `settingswidget` in clock, cpumonitor,
  memorymonitor, volume (`.ui`) and windowlist, plus sensors'
  `WidgetSensorConf`. Each applet's context-menu "Settings" action becomes
  `forest-settings desktop/panel/applets/<name>`.
- Redesign each page's contents while porting it, not as a 1:1 copy of
  the old dialog.
- Settings files (`"CPU Monitor"`, `"Window List"`, `"Temperature
  Monitor"`, …) can stay as they are for now. Multiple panels (#53) will
  need per-instance applet settings, which is the time to move them.

## Theming

`base/settings.css` needs rules for the tree view, the search field and
the search-match highlight. Per-theme `settings.css` overrides (#56) are
independent of this plan.

## Order of work

1. **Interface, loading, categories, tree, deep links.** One PR, since the
   interface change breaks every settings plugin at once: port all six
   plugins, move install paths, add the config migration.
2. **Search.**
3. **Single instance** (`org.forest.Settings`).
4. **Applet settings.** One PR for the interface and the panel-settings
   wiring, ported applets in the same or follow-up PRs.

## Open questions

- **Applets not on the panel.** Show settings pages for every installed
  applet (so one can be set up before it's added), or only for applets
  currently on the panel? Revisit with multiple panels (#53), where it
  becomes "which panel's clock".
- **Single-page categories.** Should Displays/Notifications still expand
  to show their one page, for consistency?
