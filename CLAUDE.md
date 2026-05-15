# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```sh
cmake -B build
cmake --build build
sudo cmake --install build
```

There are no tests. Build a single target with:
```sh
cmake --build build --target forest-logout   # or any other target name
```

## Architecture

Forest is a Qt6/C++ desktop environment for Linux (targets Debian Trixie). All UI is styled via QSS (Qt Style Sheets) — there is no QML anywhere in the project.

### Process Model

The desktop session runs as several independent processes:

- **`forest`** — the main process. Loads app plugins at startup, registers `org.forest` on DBus, and applies the global stylesheet.
- **`forest-session`** — session manager. Launches the WM (xfwm4) and autostart entries. Exec'd by `startforest` which is the xsession entry point.
- **`forest-logout`** — standalone fullscreen dialog for power actions.
- **`forest-settings`** — standalone settings app. Loads settings plugins for its sidebar panels.

There is a **two-tier plugin system** used by both `forest` and the panel.

### App Plugins (MODULE libs → `/usr/lib/forest/`)

Loaded by the `forest` main process at startup via `QPluginLoader`. Each implements `app_plugin_interface` (`library/pluginutills/app_plugin_interface.h`):

```cpp
virtual void setupPlug() = 0;
virtual void XcbEventFilter(xcb_generic_event_t *event) = 0;
virtual bool needs_xcb_events() = 0;
```

Declare with `Q_DECLARE_INTERFACE(app_plugin_interface, "forest.app.plugin.interface")` and `Q_PLUGIN_METADATA(IID "forest.app.plugin.interface")`.

Current app plugins: `desktop-app`, `panel-app`, `services-app`.

Which plugins are loaded is controlled by `QSettings("Forest","Forest")` under the `plugins/` group. Plugin `.so` files follow the naming pattern `lib<name>-app.so`.

### Panel Plugins (MODULE libs → `/usr/lib/forest/panel/`)

Loaded by `panel-app` at runtime. Each implements `panelpluginterface` (`panel/panel-library/panelpluginterface.h`):

```cpp
virtual void setupPlug(QBoxLayout *, QList<pmenuitem*>) = 0;
virtual void closePlug() = 0;
virtual void XcbEventFilter(xcb_generic_event_t *event) = 0;
virtual QHash<QString, QString> getpluginfo() = 0;
```

### Settings Plugins

`forest-settings` has the same plugin pattern for its settings panels, using `settings_plugin_interface` from `library/pluginutills/`.

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
- **`miscutills`** — wallpaper scaling, DBus helpers, color utilities, `RunOnce`
- **`pluginutills`** — plugin path resolution for both plugin types
- **`xcbutills`** — XCB helpers (numlock state)
- **`panel-library`** — shared widgets and interfaces for panel plugins (`panelpluginterface`, `PanelButton`, `GraphWidget`)

Helper CMake functions in `cmake/ForestDeps.cmake` (e.g. `forest_link_flogger(target)`) link these static libs.

### Install Layout

Mirrors the final system layout under `build/`:
- Executables → `usr/bin/`
- App/service plugins → `usr/lib/forest/`
- Panel plugins → `usr/lib/forest/panel/`
- Static libs → `usr/lib/`
- Config → `etc/forest/`
- Data/themes/wallpapers → `usr/share/forest/`

### Settings Storage

All components use `QSettings("Forest", "Forest")` — this maps to `~/.config/Forest/Forest.conf` on the user's system.
