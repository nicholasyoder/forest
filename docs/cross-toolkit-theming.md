# Cross-Toolkit Theming (research)

Research notes (2026-10) on making third-party Qt, GTK and other apps match
Forest/Biome. Not scheduled; see "Direction" for the decisions so far.

Today Forest's QSS themes (`usr/share/forest/themes/`) only style Forest's own
processes. The `usr/share/themes/Forest*/` dirs are GTK2 `gtkrc` + Openbox
themes: GTK2 is obsolete, so they aren't a useful starting point.

## Direction

- **Colors first:** light/dark (and an accent) reaching every toolkit.
- **Control shapes later:** a real Qt style + GTK3/GTK4 stylesheets.
- **libadwaita: colors only.** The other options are recorded below.
- **One source of truth:** define design tokens (colors, radii, spacing,
  borders) once and generate Forest QSS, the Qt palette/style constants,
  GTK3/GTK4 SCSS variables, libadwaita color overrides and Biome's decoration
  colors from them, so Round/Circle × Dark/Light stay in sync.

## Qt

Three layers; QSS only covers part of one:

1. **QStyle** draws every control (Fusion, Breeze, Kvantum are QStyles).
2. **QPalette** holds the colors the style draws with.
3. **QPlatformTheme plugin** is loaded by every Qt app
   (`QT_QPA_PLATFORMTHEME`) and supplies style, palette, fonts, icon theme,
   color scheme and file dialogs. KDE (plasma-integration), LXQt
   (lxqt-qtplugin) and qt6ct work this way.

Why QSS is the weak option for other people's apps:

- It has to be injected into every app somehow, and conflicts with apps that
  call `setStyleSheet` themselves.
- Styling is all-or-nothing per control: style part of a scrollbar and Qt
  stops drawing the rest natively. Some widgets are poorly supported (tree
  branch indicators, combo popups, dock widgets). It also costs performance.
- QtQuick/QML apps (many KDE apps) ignore QSS. They only follow a QStyle via
  `qqc2-desktop-style`.

Options for the control-shape stage:

- **`QProxyStyle` over Fusion** (C++), overriding only the controls that
  should differ. Full control, no extra dependency. Preferred.
- **Kvantum theme**: SVG + config driving an existing Qt style engine.
  Faster to start, less precise, adds a dependency.

Either way, deliver it with a **Forest platform-theme plugin**, which Wayland
Forest likely needs anyway (fonts, icons, color scheme, maybe file dialogs;
see the roadmap's icon theme setting). Forest's own apps could then drop much
of their QSS for standard controls and keep QSS for Forest-specific widgets
(panel, desk buttons, etc).

For the colors stage, the platform theme only needs to provide a palette and
color scheme (Fusion draws with the palette).

## GTK

- **GTK2** had C theme engines. **GTK3 removed them**: GTK3 and GTK4 themes
  are pure CSS plus image assets. No engine code is needed.
- Themes are usually SCSS with color/size variables compiled to CSS (Adwaita,
  Arc, adw-gtk3). Start by forking Adwaita's SCSS or adw-gtk3 rather than
  from scratch.
- GTK3 and GTK4 use different CSS node names, so it's effectively two
  stylesheets to maintain.
- Plain GTK3/GTK4 apps use the theme named by `gtk-theme-name` (gsettings /
  `settings.ini` / portal).

### libadwaita

Most GNOME apps now use libadwaita, which builds in its own stylesheet and
ignores the GTK theme name. Options, most to least supported:

1. **Officially supported:** color scheme (dark/light), accent color
   (libadwaita 1.6+) and contrast, read from the xdg-desktop-portal Settings
   interface. **This is the chosen approach.**
2. **`~/.config/gtk-4.0/gtk.css` recoloring:** layered on top of libadwaita's
   stylesheet; redefining its named colors (`window_bg_color`,
   `headerbar_bg_color`, etc.) is fairly robust. Unsupported but low-risk.
3. **Shape/layout overrides** in that same file (or `GTK_THEME`): fragile and
   unsupported; maintainers report widgets becoming hard to see or invisible.
4. **Patched libadwaita:** Linux Mint forked it to restore theming. Only
   realistic at the distro level, and an ongoing maintenance burden.

## Everything else

- **Titlebars** are probably the most visible inconsistency. GTK/libadwaita
  apps draw their own (headerbars) and won't take Biome's. Qt apps can use
  server-side decorations if Biome offers them via `xdg-decoration`. See the
  roadmap item on theming Biome's decorations from Forest themes.
- **Electron/Chromium, Firefox** mostly read color scheme and accent from the
  portal/GTK settings. Page content is the app's own.
- **Flatpak**: host themes aren't visible in the sandbox. A GTK theme must be
  published as a Flatpak extension (`org.gtk.Gtk3theme.*`); Qt styles
  likewise.
- **Icons, cursors, fonts** follow freedesktop/fontconfig standards across
  toolkits, so they're cheap.
- **Delivering settings on Wayland:** GTK and Qt both read `color-scheme`,
  `accent-color` and `contrast` (`org.freedesktop.appearance`) from the
  portal Settings interface. Forest needs to supply these: a portal backend,
  or gsettings plus xdg-desktop-portal-gtk. That's the standard interface, so
  it fits the Forest/Biome decoupling goal.

## Achievable tiers

| Tier | Coverage | Cost |
|---|---|---|
| Colors (palette, accent, dark/light) | Nearly everything, libadwaita included | Low |
| Control shapes/metrics | Qt Widgets, GTK3, plain GTK4; partial for QtQuick; libadwaita only via fragile overrides | Medium–high, ongoing (mostly GTK) |
| Pixel-identical | Not achievable on Linux | — |

## Sources

- [Why and how libadwaita prevents theming (GNOME Discourse)](https://discourse.gnome.org/t/why-and-how-libadwaita-prevents-theming/17079/31)
- [libadwaita #215: adw_init overriding the stylesheet](https://gitlab.gnome.org/GNOME/libadwaita/-/issues/215)
- [Linux Mint forks libadwaita to add theme support (OSNews)](https://www.osnews.com/story/142412/linux-mint-forks-gnomes-libadwaita-to-add-theme-support/)
- [gnome theming is not dead](https://tgrush.bearblog.dev/gnome-theming-is-not-dead/)
- [xdg-desktop-portal 1.18 accent color (Phoronix)](https://www.phoronix.com/news/Flatpak-1.18-Released)
- [GtkSettings:gtk-accent-color](https://docs.gtk.org/gtk4/property.Settings.gtk-accent-color.html)
- [Qt qxdgdesktopportaltheme.cpp](https://contribute.qt-project.org/doc/d8/db1/qxdgdesktopportaltheme_8cpp_source.html)
- [Chromium dark_mode_manager_linux.cc](https://chromium.googlesource.com/chromium/src.git/+/main/ui/linux/dark_mode_manager_linux.cc)
