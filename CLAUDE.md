# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

Build into `build/Desktop-Debug` (not a bare `build/`) — always build there,
and never `rm -rf` it (the user may use Qt Creator on it again). Building there
and running the staging script below is pre-approved: do both whenever a change
should be tested live, then ask the user to restart the session (never restart
it yourself).

```sh
cmake -B build/Desktop-Debug
cmake --build build/Desktop-Debug
```

`build/install_to_staging.sh` (gitignored, not part of the CMake build
itself) installs a `build/Desktop-Debug` build into `/opt/forest-build/usr/`,
which `/usr/bin/forest`, `/usr/lib64/forest`, `/usr/share/forest`, and
`/usr/share/wallpapers/forest` all symlink into — so it needs no `sudo`.
Use it (run from `build/`, after a full build) to stage changes for testing
rather than copying files into `/opt/forest-build` by hand. It always ends with
a `file INSTALL cannot set permissions on "/etc/forest"` error: that step needs
root and runs last, so it's expected and harmless unless something under
`etc/forest/` or `etc/pam.d/` actually changed (then the user has to install
that part manually). Keep new `install()` rules above the `/etc` ones in the
top-level `CMakeLists.txt`, with prefix-relative destinations. Systemd units and
the polkit policy reach the live system only through hand-made symlinks
(`/usr/lib/systemd/user/forest-session.target`,
`/usr/share/polkit-1/actions/org.forest.login.policy`); a new file there needs
one too.

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

## Releases

See `docs/release-checklist.md` for everything to update when preparing a Forest + Biome release.

## Debian Packaging

`debian/` is kept in the repo for a possible return to Debian, but Fedora
(`packaging/forest.spec`) is the real target. When a change affects packaging
(new binaries or files, dependencies, man pages), update `debian/` alongside the
spec, best effort. Don't build or test Debian packages; leave `debian/changelog`
for release time.

## Architecture

Forest is a Qt6/C++ desktop environment for Linux (targets Fedora; Debian Trixie through 0.10.0). All UI is styled via QSS (Qt Style Sheets) — there is no QML anywhere in the project.

### Process Model

The desktop session runs as several independent processes:

- **`forest`** — the main process. Claims `org.forest` first (exits 0 if already owned), then loads app plugins and applies the global stylesheet. Restarted by `forest-session` (`SupervisedProcess`, backoff, min delay 1 s) unless it exits 0.
- **`forest-session`** — session manager. Launches autostart entries, and supervises `forest` and `forest-locker`. Forked by the Biome compositor (its `-s` flag) once Biome's Wayland socket is ready — Biome is the top-level process, exec'd by `startforest-wayland`, the wayland-sessions entry point.
- **`forest-logout`** — standalone fullscreen dialog for power actions.
- **`forest-locker`** — idle/lock daemon supervised by `forest-session`: dim, display power, logind lock/sleep, `org.freedesktop.ScreenSaver`. Runs **`forest-lockscreen`** (`ext-session-lock-v1` + PAM) to lock. Split in two (like swayidle + swaylock) so a crashing lock UI can't leave the session stuck locked or the displays dark; the daemon respawns it. Deliberately no `org.forest.Locker`: locking goes only through logind `Session.Lock`, `org.freedesktop.ScreenSaver` and `loginctl lock-session`, so other lockers work with Forest and vice versa.
- **`forest-settings`** — standalone settings app. Loads every settings plugin in `<libdir>/forest/settings/` and shows their pages in a category tree. Single instance: owns `org.forest.Settings`; later launches forward their path and `XDG_ACTIVATION_TOKEN` to `OpenPage` and exit.

There is a **two-tier plugin system** used by both `forest` and the panel.

### App Plugins (MODULE libs → `<libdir>/forest/`)

Loaded by the `forest` main process at startup via `QPluginLoader`. Each implements `app_plugin_interface` (`library/pluginutills/app_plugin_interface.h`):

```cpp
virtual void setupPlug() = 0;
```

The interface is declared as `"forest.app.plugin.interface"`; each plugin's `Q_PLUGIN_METADATA` IID is currently its own string (`forest.app.panel.plugin`, …), so loaders identify plugins by `qobject_cast`, not IID.

Current app plugins: `desktop-app`, `panel-app`, `services-app`.

Which plugins are loaded is controlled by `QSettings("Forest","Forest")` under the `plugins/` group. Plugin `.so` files follow the naming pattern `lib<name>-app.so`.

### Panel Plugins (MODULE libs → `<libdir>/forest/panel/`)

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

MODULE libs → `<libdir>/forest/settings/`, all loaded (no config entry). Each implements `settings_plugin_interface` (`library/pluginutills/settings_plugin_interface.h`), returning a flat list of `settings_page`s. A page's path (`desktop/panel`) places it: the first segment is a category ID from the table in `settings/settingsmanager.cpp`, the rest its parent page. Paths double as deep links (`forest-settings desktop/panel`). Pages load/save controls with `SettingsBinder` (`settings/widgets`) against keys + defaults from a shared `<component>config.h`; rows follow their control's `setVisible` / `setEnabled`.

### Theming System

Themes live in `/usr/share/forest/themes/<ThemeName>/`. Each theme has:
- `theme.conf` — declares `name`, `description`, `hidden`, optionally `parent_themes` (comma-separated list), and a `[Palette]` group
- Per-component CSS files: `forest.css` (the `forest` process), `settings.css`, `login.css` (greeter + lockscreen), `logout.css`, `biome.css` (Biome decorations/switcher), plus `shared.css`, appended for every Forest process and Biome (menus etc.)

Parsing the theme format (`parent_themes` chain, `[Palette]` → `QPalette`, per-layer CSS concatenation) lives in the header-only `library/forest-theme` (`foresttheme.h`), installed as the `ForestTheme` CMake package (`forest-theme-devel`) so Biome builds against the same code; add new palette keys there. `fstyleloader` wraps it with Forest's specifics (QSettings theme lookup, `ForestOutlineStyle`).

`fstyleloader::apply("componentname")` (in `library/fstyleloader/fstyleloader.h`) sets `ForestOutlineStyle`, the palette, and the concatenated `<component>.css` + `shared.css`. The active theme is read from `QSettings("Forest","Forest")` key `theme`. `forest-screenshot` applies only the palette (`applypalette()`), no QSS.

Theme inheritance example — "Round-Dark" has `parent_themes=base,base-rounded,base-dark`, so it loads `base/forest.css` → `base-rounded/forest.css` → `base-dark/forest.css` → `Round-Dark/forest.css`, then `base/shared.css` → … → `Round-Dark/shared.css`. Files are the outer loop, so every layer's `shared.css` comes after every layer's component CSS: a later layer can't override a parent's `shared.css` rule from its component file. Adding a new component means adding a `<component>.css` to `base/` (and overrides in variant themes as needed).

Biome reads the same layers (`biome.css` + `shared.css`) through `etc/biome/conf.d/50-forest.conf` (`[Theme] dir/name`); the theme page also writes `[Theme] name` to the user's `Biome.conf` so decorations follow live.

Themes are selected from user-visible themes (those without `hidden=True`): `Circle-Dark`, `Circle-Light`, `Round-Dark`, `Round-Light`.

### DBus

`forest` owns `org.forest` on the session bus. `forest` and its plugins register objects under it: `/org/forest` (`loadstylesheet`), `/org/forest/panel` (+ `/mainmenu`, `/quicklaunch`, `/volume` from applets), `/org/forest/desktop`, `/org/forest/hotkeys`, `/org/forest/showdesktop`, `/org/forest/displays` (interface `org.forest.displays`). Most use `ExportAllSlots`, so inherited `QWidget` slots are exported too; treat only the slots callers use as API. Settings pages call them via `miscutills::call_dbus("forest/panel/reloadplugins")` (object `/org/forest/panel`, slot `reloadplugins`; blocking).

### Shared Libraries (static, built first)

All in `library/` (except `panel-library`). They're linked into each plugin module separately, so a "singleton" in one is one per module, not per process:
- **`forest-theme`** — header-only theme format parser, installed for Biome (see Theming System)
- **`fstyleloader`** — theme CSS loading (header-only implementation in `.h`)
- **`flogger`** — logging setup; call `FLogger::install("appname")` at startup
- **`miscutills`** — wallpaper scaling, DBus helpers, color utilities, `RunOnce`, `ScreenTracker` (screen-change tracking + `primary()` screen)
- **`pluginutills`** — app plugin path resolution, settings plugin interface
- **`menuanchor`** — xdg_positioner placement for `QMenu`s (see Menus above)
- **`toplevels`** — wlr-foreign-toplevel, ext-foreign-toplevel-list and ext-workspace clients, `WorkspaceTracker` (ext-workspace + `org.biome.Workspaces`, keyed by workspace `id`), `ToplevelTracker` (open windows + which are on the active workspace), and `ToplevelCapture`/`ScreenGrab` (ext-image-copy-capture window and output capture)
- **`activation`** — xdg-activation client (`XdgActivation`): launch programs with a token so they can raise an existing window. `QMenu` actions need `watch()`; custom popups call `request()` before hiding. Create the instance at startup (binds asynchronously)
- **`outputs`** — wlr-output-management client (`OutputManager`), display profiles (`DisplayProfiles`, `Displays.conf`) and layout fixups; see `docs/development-notes.md` → Display settings
- **`hotkeyconfig`** — parse/format of `[hotkeys]` `DBUS:` actions, plus the Hotkeys page's built-in actions
- **`layeroverlay`** — solid-colour layer-shell overlays (startup black, locker/logout dim) without a `QWindow`
- **`sessionlock`** — ext-session-lock-v1 shell integration (lockscreen)
- **`loginui`** — greeter/lockscreen widgets (`PasswordForm`, `ScreenSet`, wallpaper, avatar)
- **`fileops`** — threaded copy/move/delete/trash with conflict dialogs (desktop)
- **`panel-library`** (`panel/panel-library`) — shared widgets and interfaces for panel plugins (`panelpluginterface`, `panelbutton`, `popup`, `panelanchor`, `graphwidget`)

Helper CMake functions in `cmake/ForestDeps.cmake` (e.g. `forest_link_flogger(target)`) link these static libs.

### Install Layout

Mirrors the final system layout under `build/`:
- Executables → `usr/bin/`
- App/service plugins → `usr/lib64/forest/`
- Panel plugins → `usr/lib64/forest/panel/`
- Settings plugins → `usr/lib64/forest/settings/`
- Config → `etc/forest/`
- Data/themes/wallpapers → `usr/share/forest/`

### Settings Storage

All components use `QSettings("Forest", "Forest")` — this maps to `~/.config/Forest/Forest.conf` on the user's system.
