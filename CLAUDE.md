# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

Qt Creator is configured to build this project into `build/Desktop-Debug`
(not a bare `build/`) — always build there, and never `rm -rf` it, since
Qt Creator owns it as a live, incrementally-updated build directory.

```sh
cmake -B build/Desktop-Debug
cmake --build build/Desktop-Debug
```

`build/install_to_staging.sh` (gitignored, not part of the CMake build
itself) installs a `build/Desktop-Debug` build into `/opt/forest-build/usr/`,
which `/usr/bin/forest`, `/usr/lib/forest`, `/usr/share/forest`, and
`/usr/share/wallpapers/forest` all symlink into — so it needs no `sudo`.
Use it (run from `build/`, after a full build) to stage changes for testing
rather than copying files into `/opt/forest-build` by hand. It always ends with
a `file INSTALL cannot set permissions on "/etc/forest"` error: that step needs
root and runs last, so it's expected and harmless unless something under
`etc/forest/` or `etc/pam.d/` actually changed (then the user has to install
that part manually). Keep new `install()` rules above the `/etc` ones in the
top-level `CMakeLists.txt`, with prefix-relative destinations.

There are no tests. Build a single target with:
```sh
cmake --build build/Desktop-Debug --target forest-logout   # or any other target name
```

## Git Workflow

Same model in `forest` and `biome`:

- **Major changes:** branch off an up-to-date `develop`, implement and test there, push, and open a PR into `develop` (`gh pr create --base develop`). The user merges PRs and pulls `develop` locally — don't do either.
- **Small tweaks** (roadmap/doc edits, other small standalone doc changes, a code change of a couple of lines): commit straight to `develop` and push — unless a PR is about to be opened, in which case put it on that branch instead.
- `master` only changes at a release (`develop` merged in and tagged by the user). Never commit to or PR against it.
- **Commit once a change is settled, not on every edit.** If you've asked the user to verify something or answer a question that could change the edit, leave it uncommitted until they reply. Then fold the follow-up into the same commit.

## Architecture

Forest is a Qt6/C++ desktop environment for Linux (targets Debian Trixie). All UI is styled via QSS (Qt Style Sheets) — there is no QML anywhere in the project.

### Process Model

The desktop session runs as several independent processes:

- **`forest`** — the main process. Loads app plugins at startup, registers `org.forest` on DBus, and applies the global stylesheet.
- **`forest-session`** — session manager. Launches autostart entries and the `forest` main process. Forked by the Biome compositor (its `-s` flag) once Biome's Wayland socket is ready — Biome is the top-level process, exec'd by `startforest-wayland`, the wayland-sessions entry point.
- **`forest-logout`** — standalone fullscreen dialog for power actions.
- **`forest-locker`** — idle/lock daemon started by `forest-session`: dim, display power, logind lock/sleep, `org.freedesktop.ScreenSaver`. Runs **`forest-lockscreen`** (`ext-session-lock-v1` + PAM) to lock. Split in two (like swayidle + swaylock) so a crashing lock UI can't leave the session stuck locked or the displays dark; the daemon respawns it. Deliberately no `org.forest.Locker`: locking goes only through logind `Session.Lock`, `org.freedesktop.ScreenSaver` and `loginctl lock-session`, so other lockers work with Forest and vice versa.
- **`forest-settings`** — standalone settings app. Loads every settings plugin in `/usr/lib/forest/settings/` and shows their pages in a category tree. Single instance: owns `org.forest.Settings`; later launches forward their path and `XDG_ACTIVATION_TOKEN` to `OpenPage` and exit.

There is a **two-tier plugin system** used by both `forest` and the panel.

### App Plugins (MODULE libs → `/usr/lib/forest/`)

Loaded by the `forest` main process at startup via `QPluginLoader`. Each implements `app_plugin_interface` (`library/pluginutills/app_plugin_interface.h`):

```cpp
virtual void setupPlug() = 0;
```

Declare with `Q_DECLARE_INTERFACE(app_plugin_interface, "forest.app.plugin.interface")` and `Q_PLUGIN_METADATA(IID "forest.app.plugin.interface")`.

Current app plugins: `desktop-app`, `panel-app`, `services-app`.

Which plugins are loaded is controlled by `QSettings("Forest","Forest")` under the `plugins/` group. Plugin `.so` files follow the naming pattern `lib<name>-app.so`.

### Panel Plugins (MODULE libs → `/usr/lib/forest/panel/`)

Loaded by `panel-app` at runtime. Each implements `panelpluginterface` (`panel/panel-library/panelpluginterface.h`):

```cpp
virtual void setupPlug(QBoxLayout *, QList<QAction*>) = 0;
virtual void closePlug() = 0;
virtual void reloadSettings() {}
```

Applet info is JSON metadata (`Q_PLUGIN_METADATA(... FILE "clock.json")`: `name`, optional `settings` page path and `stretch`), read with `QPluginLoader::metaData()` so panel-settings never instantiates applets. The `QAction` list is panel-provided context-menu items, added first: "Panel Settings", then (when `settings` is set) a separator and "<Applet> Settings", which the applet's own items follow without a separator.

Applet settings pages are a separate `<applet>-settings` module (`forest_add_applet_settings()` in `cmake/ForestDeps.cmake`) sharing keys with the applet through `<applet>config.h`. After saving, the page calls `forest/panel/reloadappletsettings` with its path, which runs `reloadSettings()` on that applet only.

### Menus

All menus are `QMenu`, styled by the theme's `QMenu` rules. Qt can't place popups off layer surfaces (it thinks the panel/desktop sit at (0,0)), so open them via `popupMenuOnLauncher()` (`panelanchor.h`, panel launchers) or `menuanchor::anchorMenuAtPoint()` (`library/menuanchor`), never a bare `popup(pos)`. `menuanchor::MenuFilter`, installed app-wide in `forest.cpp`, handles translucency and submenu placement.

### Settings Plugins

MODULE libs → `/usr/lib/forest/settings/`, all loaded (no config entry). Each implements `settings_plugin_interface` (`library/pluginutills/settings_plugin_interface.h`), returning a flat list of `settings_page`s. A page's path (`desktop/panel`) places it: the first segment is a category ID from the table in `settings/settingsmanager.cpp`, the rest its parent page. Paths double as deep links (`forest-settings desktop/panel`). Pages load/save controls with `SettingsBinder` (`settings/widgets`) against keys + defaults from a shared `<component>config.h`; rows follow their control's `setVisible` / `setEnabled`.

### Theming System

Themes live in `/usr/share/forest/themes/<ThemeName>/`. Each theme has:
- `theme.conf` — declares `name`, `description`, `hidden`, and optionally `parent_themes` (comma-separated list)
- Per-component CSS files: `forest.css`, `logout.css`, etc.

`fstyleloader::loadstyle("componentname")` (in `library/fstyleloader/fstyleloader.h`) walks the `parent_themes` chain and concatenates each layer's CSS. The active theme is read from `QSettings("Forest","Forest")` key `theme`.

Theme inheritance example — "Round-Dark" has `parent_themes=base,base-rounded,base-dark`, so it loads `base/forest.css` → `base-rounded/forest.css` → `base-dark/forest.css` → `Round-Dark/forest.css`. Adding a new component means adding a `<component>.css` to `base/` (and overrides in variant themes as needed).

Themes are selected from user-visible themes (those without `hidden=True`): `Circle-Dark`, `Circle-Light`, `Round-Dark`, `Round-Light`.

### DBus

`forest` registers `org.forest` / `/org/forest` on the session bus and exports all slots. Other components trigger actions (e.g. stylesheet reload) via `miscutills::call_dbus("forest/<slot-name>")`.

### Shared Libraries (static, built first)

All in `library/`:
- **`fstyleloader`** — theme CSS loading (header-only implementation in `.h`)
- **`flogger`** — logging setup; call `FLogger::install("appname")` at startup
- **`miscutills`** — wallpaper scaling, DBus helpers, color utilities, `RunOnce`, `ScreenTracker` (screen-change tracking + `primary()` screen)
- **`pluginutills`** — app plugin path resolution, settings plugin interface
- **`menuanchor`** — xdg_positioner placement for `QMenu`s (see Menus above)
- **`toplevels`** — wlr-foreign-toplevel, ext-foreign-toplevel-list and ext-workspace clients, `BiomeWorkspaces`, and `ToplevelTracker` (open windows + which are on the active workspace)
- **`activation`** — xdg-activation client (`XdgActivation`): launch programs with a token so they can raise an existing window. `QMenu` actions need `watch()`; custom popups call `request()` before hiding. Create the instance at startup (binds asynchronously)
- **`outputs`** — wlr-output-management client (`OutputManager`), display profiles (`DisplayProfiles`, `Displays.conf`) and layout fixups; see `docs/development-notes.md` → Display settings
- **`hotkeyconfig`** — parse/format of `[hotkeys]` `DBUS:` actions, plus the Hotkeys page's built-in actions
- **`panel-library`** — shared widgets and interfaces for panel plugins (`panelpluginterface`, `panelbutton`, `graphwidget`)

Helper CMake functions in `cmake/ForestDeps.cmake` (e.g. `forest_link_flogger(target)`) link these static libs.

### Install Layout

Mirrors the final system layout under `build/`:
- Executables → `usr/bin/`
- App/service plugins → `usr/lib/forest/`
- Panel plugins → `usr/lib/forest/panel/`
- Settings plugins → `usr/lib/forest/settings/`
- Static libs → `usr/lib/`
- Config → `etc/forest/`
- Data/themes/wallpapers → `usr/share/forest/`

### Settings Storage

All components use `QSettings("Forest", "Forest")` — this maps to `~/.config/Forest/Forest.conf` on the user's system.
