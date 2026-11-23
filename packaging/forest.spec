Name:           forest
Version:        0.11.2
Release:        1%{?dist}
Summary:        Lightweight Qt-based desktop environment

License:        LGPL-3.0-or-later
URL:            https://github.com/nicholasyoder/forest
Source0:        %{url}/archive/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  cmake
BuildRequires:  pkgconfig
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtbase-private-devel
BuildRequires:  layer-shell-qt-devel
BuildRequires:  libdbusmenu-lxqt-devel
BuildRequires:  polkit-qt6-1-devel
BuildRequires:  libqtxdg-devel
BuildRequires:  libXcursor-devel
BuildRequires:  alsa-lib-devel
BuildRequires:  lm_sensors-devel
BuildRequires:  pam-devel
BuildRequires:  libxkbcommon-devel
BuildRequires:  wayland-devel
BuildRequires:  wayland-protocols-devel >= 1.40
BuildRequires:  systemd-rpm-macros

Requires:       biome >= 0.3.1
Requires:       xdg-desktop-portal
Requires:       dbus-tools
Requires:       gtk-murrine-engine
Requires:       qt6ct
Requires:       papirus-icon-theme
Requires:       brightnessctl
Requires:       xdg-user-dirs
Requires:       /usr/bin/gsettings
Requires:       qterminal
Requires:       pcmanfm-qt
Requires:       polkit
Requires:       jetbrains-mono-fonts
Requires:       dejavu-sans-fonts

%description
Forest is a desktop environment built using Qt, running on the Biome
Wayland compositor. It is still very much a work in progress so expect
to encounter partially implemented features.

%package greeter
Summary:        greetd greeter using the Forest theme system
Requires:       %{name} = %{version}-%{release}
Requires:       greetd
Requires:       biome >= 0.3.1
Recommends:     xorg-x11-xinit
Recommends:     xorg-x11-server-Xorg

%description greeter
A Qt-based login greeter for the greetd display manager, styled with the
Forest theme system. Runs inside the Biome Wayland compositor on VT7.
xorg-x11-xinit and xorg-x11-server-Xorg are only needed to start xsessions
entries (see forest-greeter-xsession); Forest's own session is Wayland-only.

%package theme-devel
Summary:        Build-time theme loader library shared by Forest and Biome
Requires:       cmake(Qt6Gui)

%description theme-devel
Header-only library (ForestTheme CMake package) that reads Forest's theme
format: the parent_themes chain, the [Palette] group and per-layer CSS.
Biome builds against it to style its decorations from Forest themes.

%prep
%autosetup

%build
%cmake -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
%cmake_install
install -d %{buildroot}%{_sharedstatedir}/forest/login
install -d %{buildroot}%{_mandir}/man1
install -pm 0644 man/*.1 %{buildroot}%{_mandir}/man1/

%files
%license LICENSE
%{_bindir}/forest
%{_bindir}/forest-session
%{_bindir}/forest-logout
%{_bindir}/forest-settings
%{_bindir}/forest-locker
%{_bindir}/forest-lockscreen
%{_bindir}/forest-screenshot
%{_libdir}/forest/
%dir %{_sysconfdir}/forest
%config(noreplace) %{_sysconfdir}/forest/Forest.conf
%config(noreplace) %{_sysconfdir}/forest/Panel.conf
%config(noreplace) %{_sysconfdir}/forest/Quicklaunch.conf
%config(noreplace) %{_sysconfdir}/pam.d/forest-locker
%config(noreplace) %{_sysconfdir}/biome/conf.d/50-forest.conf
%{_datadir}/wayland-sessions/Forest.desktop
%{_datadir}/applications/forest-screenshot.desktop
%{_datadir}/applications/forest-settings.desktop
%{_datadir}/applications/org.forest.Forest.desktop
%{_datadir}/forest/
%exclude %{_datadir}/forest/forest-greeter-xsession
%{_datadir}/wallpapers/forest/
%{_datadir}/themes/Forest/
%{_datadir}/polkit-1/actions/org.forest.login.policy
%{_userunitdir}/forest-session.target
%dir %{_sharedstatedir}/forest
%dir %{_sharedstatedir}/forest/login
%ghost %config(noreplace) %{_sysconfdir}/forest/Login.conf
%{_mandir}/man1/forest.1*
%{_mandir}/man1/forest-session.1*
%{_mandir}/man1/forest-logout.1*
%{_mandir}/man1/forest-settings.1*
%{_mandir}/man1/forest-locker.1*
%{_mandir}/man1/forest-lockscreen.1*
%{_mandir}/man1/forest-screenshot.1*

%files theme-devel
%{_includedir}/forest-theme/
%{_datadir}/cmake/ForestTheme/

%files greeter
%license LICENSE
%{_bindir}/forest-greeter
%{_datadir}/forest/forest-greeter-xsession
%config(noreplace) %{_sysconfdir}/forest/greetd.toml
%dir %{_unitdir}/greetd.service.d
%{_unitdir}/greetd.service.d/50-forest.conf
%{_mandir}/man1/forest-greeter.1*

%changelog
* Mon Nov 23 2026 Nicholas Yoder <nicholasyoder@proton.me> - 0.11.2-1
- Remove the unused `nmcontrol` panel plugin stub, a stale generated header and the `desktop::reloadsettings` D-Bus method
- Greeter and logout fixes: greetd replies are matched to requests in order, the greeter lists users through NSS/SSSD and sessions through `XdgDesktopFile`, and the logout dialog no longer crashes on an unknown saved action or reports a slow polkit prompt as a failure
- Match workspaces to Biome's by their ext-workspace `id` instead of arrival order (falling back to arrival order if none is sent)
- Rework notification popups: `expire_timeout` 0 now never expires, `replaces_id` updates a popup in place, `CloseNotification` on an unknown id returns an error, and at most 5 popups show at once
- Render `<b>`, `<i>`, `<u>`, `<br>` and clickable http/https/mailto links in notification bodies (`body-markup`)
- Applet fixes: the battery warning no longer fires while charging and the applet hides without a battery; the CPU monitor no longer double-counts guest time; the temperature applet reads only displayed sensors and handles none; Quicklaunch keeps every file of a multi-file drop; the tray honours `ItemIsMenu`; mute toggles the playback switch instead of zeroing volume
- Settings and displays fixes: a background change no longer jumps the settings window to its page; the Cursor page no longer rewrites `index.theme` on every keystroke; About and the Displays page no longer run shell commands or connect to the compositor up front; the Panel page keeps applets it can't load; the displays service re-applies the saved profile when a monitor reconnects off its layout
- Screenshot fixes: the PNG is encoded once for the file and clipboard, the clipboard copy is reported only once the helper owns the selection, the helper prefers `ext-data-control-v1` over `wlr-data-control`, and a capture with no screens fails instead of hanging

* Tue Nov 17 2026 Nicholas Yoder <nicholasyoder@proton.me> - 0.11.1-1
- Packaging cleanup: drop the unneeded `qt6-qtwayland` and `gio` dependencies, the leftover Openbox themes, `Forest-Flat-Dark`, qt5ct seeding and `power.png`; own the greetd drop-in directory, `/var/lib/forest/login` and `Login.conf`; install a `ForestThemeConfigVersion.cmake` so `find_package(ForestTheme <version>)` works; the session prepends Flatpak dirs to `XDG_DATA_DIRS` instead of replacing it, and stops `forest-session.target` when Biome exits
- Default to JetBrains Mono as the monospace font and `qterminal` as the terminal (Meta+Q, Quicklaunch)
- Fix applets with a settings gear in the Panel settings list not toggling on or off: the gear is now painted by the list's delegate (styled by `#AppletSettingsButton`, with hover and pressed states) instead of a widget laid over the row
- Fix applet reloads (adding, removing or reordering panel applets) leaking running timers, popups, the whole main menu and its `.desktop` watcher; the windowlist preview of a window now closes with that window; the clock only repaints (and wakes) on the second/minute boundary
- Fix a memory leak of about 230 MB (two 4K screens) in `forest` on every wallpaper change: the wallpaper is now decoded only when its file changes and scaled once per screen, and the login and lock screens scale on resize instead of on every paint
- Remove the Tile wallpaper mode (it never drew anything; existing Tile settings become Fill) and store the mode by name in the config, converted on upgrade (a custom login-screen mode is reset to Fill until re-applied); the Wallpaper settings page now saves through the shared binder, reloads the desktop without blocking, and previews the chosen mode and every supported image format
- Fix the panel being remapped whenever any monitor was plugged in, unplugged or dropped out (e.g. on a display blank); it now rebuilds only when its own screen changes, which also stops windows maximized on that screen briefly resizing
- Require `.desktop` launchers on the desktop to be marked executable: an unmarked one shows its real filename and a generic icon, and asks "Trust and launch?" (showing its command) before running and marking it executable. "Show on desktop" and dragging from the main menu create executable copies, and desktop launchers and folders now open with an xdg-activation token
- Rework hotkey capture to record the real key: F-keys, media keys and the rest of the keyboard now work, Print and Return are allowed, Caps Lock no longer breaks letters, Shift+letter shortcuts fire, and Esc cancels recording. Existing hotkeys are converted on upgrade
- Show a warning on hotkeys the compositor couldn't bind (refused or already in use) in the Hotkeys settings, and refuse plain keys without Ctrl, Alt or Meta when recording
- Stop a held repeating hotkey after 5 seconds or when the session locks
- Make the hotkey edit window modal; fix a possible crash on a malformed built-in D-Bus hotkey, and two simultaneous new hotkeys taking the same id
- Restart the `forest` shell (panel, desktop, notifications, hotkeys) if it crashes, with backoff; a second `forest` now exits instead of running alongside the first
- Fix the Displays settings page staying stuck on "pending" if the shell restarts while a layout change awaits confirmation
- Fix first-run default settings moving the whole `~/.config/Forest` aside when `Forest.conf` had no version; missing defaults are now copied in without touching existing files
- Keep the login keyring unlocked: the session now starts the Secret Service daemon at login, so it attaches to the unlocked PAM-started one before that exits after a few idle minutes (previously the first app to ask minutes later got a locked keyring and a password prompt)
- Make the screen locker fail safe: it keeps respawning the lockscreen once Biome holds a lock, shows a notification when a lock fails, no longer drops a lock requested (e.g. on suspend) just after unlocking, ignores SimulateUserActivity while locked, and can no longer hang on exit when the lock is refused
- Rewrite the polkit authentication dialog: the typed password now goes to the selected user (current user preselected) instead of looping on the wrong one with two admin accounts, Cancel closes the dialog and completes the request, concurrent requests are queued instead of hanging, and PAM messages show inline
- Fix a possible panel crash when a window resizes while its live preview is showing in the windowlist popup
- Fix a possible panel crash when a tray app exits while its menu is being opened
- Fix a possible settings crash, or an autostart command saved over another entry, when removing or adding an autostart entry right after typing in one
- Fix copying or moving a file onto a same-named folder deleting the whole folder when Replace (or Replace with Apply to all) was chosen; a folder is now never replaced by a file
- Make Replace in file copy/move safe: the new file is written to a temporary name and swapped in only on success, so a failed or cancelled copy no longer loses the original; new files and folders are created private and only get their final permissions after the data is written, and merging into an existing folder no longer changes its permissions

* Fri Nov 13 2026 Nicholas Yoder <nicholasyoder@proton.me> - 0.11.0-1
- Move theme loading (parent_themes chain, palette, per-layer CSS) into a header-only forest-theme library, installed with the new forest-theme-devel package so Biome can use it
- Move menu styling into a per-theme shared.css, also loaded by Biome for its window menu; add arrow spacing for submenu items; call virtual desktops "workspaces" in the windowlist menu
- Add a notification position setting (any screen corner), and stack simultaneous notifications instead of overlapping them, reflowing as they close
- Hold notifications that arrive during login until the startup fade-in finishes, so they no longer draw over it
- Theme the settings app from the selected desktop theme (light or dark), restyling live when the theme changes
- Draw text box, spin box, checkbox and radio button outlines in a light color on dark themes
- Replace the grim and slurp screenshot script with a native forest-screenshot: a capture dialog in the main menu (whole workspace, single monitor, active window or region, with a delay), plus --full, --monitor, --window and --region flags for hotkeys. A bare forest-screenshot now opens the dialog instead of capturing the whole screen; bind --full for that
- Show live window previews in the windowlist hover popup, including for minimized and other-workspace windows (needs Biome with per-window capture; otherwise the app icon is shown), with a preview size setting
- Drag a multi-icon selection on the desktop as a group, keeping the icons' relative positions

* Wed Nov 11 2026 Nicholas Yoder <nicholasyoder@proton.me> - 0.10.1-1
- Port to Fedora and Qt 6.11; no functional changes
- Plugins install to the distro libdir (/usr/lib64/forest on Fedora); stored panel applet paths are migrated

* Mon Nov 09 2026 Nicholas Yoder <nicholasyoder@proton.me> - 0.10.0-1
- Run the greeter on Biome instead of cage, with a window per monitor; the login card follows the focused monitor
- Show a Caps Lock warning in the greeter
- Add a login and lock screen wallpaper setting
- Rename themes' greeter.css to login.css, shared by the greeter and lockscreen
- Theme Biome's window decorations from Forest themes, restyling live on theme change
- Add a Displays settings page: arrangement, resolution, refresh rate, scale, orientation and primary display, with a keep-or-revert prompt
- Add display profiles, picked automatically when monitors are plugged in, with hotkeys to switch between them
- Keep idle-blanked displays dark when a monitor reconnects or is plugged in
- Rework the settings app: category tree, search, and a single window that raises when relaunched
- Move panel applet settings into the settings app, opened from each applet's menu
- Relaunching an app from the main menu, quicklaunch or panel raises its existing window
- Style the desktop switcher, sensor bars and CPU/memory graphs from the theme, and use theme icons for the battery monitor, so they draw cleanly at fractional scales
- Fix the CPU and memory monitor update interval setting, and swap always reading 0 when shown separately
- Add optional hold-to-repeat for hotkeys, on by default for volume and brightness
- Add a Built-in action list and a D-Bus argument field to hotkey settings
- Fix global hotkeys not binding on punctuation keys
- Show browser notifications in Forest's popups: report capabilities, show action buttons and send close and action signals
- Give panel popups, desktop icons and the logout dialog keyboard focus through standard Wayland protocols instead of Biome exceptions
- Fix a stall when showing the logout dialog on 4K displays
- Fix hotkey settings crashing when adding the first hotkey
- Fix forest-locker crashing on wake after a monitor disconnected while dimmed
- Fix the settings app crashing when a page rebuilds after being opened twice

* Mon Oct 05 2026 Nicholas Yoder <nicholasyoder@proton.me> - 0.9.0-1
- Move to Wayland: Forest now runs on the Biome compositor instead of X11 and xfwm4
- Global hotkeys through the GlobalShortcuts portal
- Allow recording a bare Meta key tap as a hotkey
- Add a screen locker (forest-locker and forest-lockscreen)
- Add a Lock button to the logout dialog and a Meta+L hotkey
- Implement org.freedesktop.ScreenSaver so apps can inhibit idle and lock the screen
- Take screenshots with grim and slurp (forest-screenshot)
- Replace the system tray with a StatusNotifierItem (SNI) tray
- Fix late-registered tray icons collapsing to zero width
- Convert panel menus to QMenus styled to match the theme, with real submenus
- Fix desktop file operations with progress, cancel, conflict prompts and trash support
- Grey out unavailable actions on the logout dialog
- End the session properly on logout
- Fix the panel disappearing during screen changes and improve wallpaper screen change tracking
- Fix panel plugin reload ignoring changes and tidy plugin display names
- Fix the battery monitor not refreshing after a panel plugin reload
- Greeter passes session environment from .desktop files to greetd
- Rename the panel separator config value, upgrading existing configs

* Tue Jul 21 2026 Nicholas Yoder <nicholasyoder@proton.me> - 0.8.0-1
- Migrate from Qt5 to Qt6 and qmake to cmake
- Add prototype greeter app for greetd
- Improve new hotkey recording so existing hotkeys don't prevent recording some combinations
- Fix button highlights on system tray icons
- Fix low resolution system tray icons
- Add controls to the settings app for configuring session autostart commands
- Allow clicking on long notifications to view full text in a scrollable window
- Quicklaunch launchers will use the proper command after the underlying desktop file changes on disk
- Panel will now properly resize and change screens after resolution and primary screen change
- Add timeout indicator to notification popups and restrict popup size
- Allow retrying after failed polkit authentication attempts
- Properly get window previews on some hidden windows
- Save volume levels

* Tue Mar 25 2025 Nicholas Yoder <nicholasyoder@proton.me> - 0.7.9-1
- Fix wallpaper fade in on launch
- Improve detection of window movements in desktop switcher
- Add 'Move to desktop' option in the window list menu
- Rewrite logout manager code
- Support settings upgrades between versions
- Add arrow to settings items containing subitems
- Improve volume control applet
- Improve multi monitor support
- Implement base themes with functional light/dark variations of Circle and Rounded themes
- Fix random pixels in tray icons

* Mon Sep 23 2024 Nicholas Yoder <nicholasyoder@proton.me> - 0.7.8-1
- Fix panel on top of fullscreen windows
- Fix library linking
- Add docs directory with changelog and development notes
- Add auto-hide functionality to panel
- Add session manager
- Rebuild the settings app
- Restructure project
- Improve notifications
- Add support for Meta single key global hotkey

* Mon Jan 30 2023 Nicholas Yoder <nicholasyoder@proton.me> - 0.7.7-1
- Start using KWindowSystem for some things
- Allow window reordering on the windowlist
- Add SNI systemtray but keep the old one available as well

* Sat Dec 17 2022 Nicholas Yoder <nicholasyoder@proton.me> - 0.7.6-1
- Fixed mainmenu name
- Added hotkeys for screen brightness and screenshot
- Enabled seperator between menu and quicklaunch

* Sat Dec 17 2022 Nicholas Yoder <nicholasyoder@proton.me> - 0.7.5-1
- (Changelog entries for the point releases between 0.7.0 and 0.7.5 were lost)
- Start tracking the project in git and keeping a changelog in the project itself
- Rebuilt mainmenu plugin
- Fix faded pixel at the top of the CPU and RAM monitor bars

* Thu May 26 2022 Nicholas Yoder <nicholasyoder@proton.me> - 0.7.0-1
- Rebuild the settings app with XML-defined pages
- Style the panel, main menu and settings app with style sheets
- Add settings for panel, themes, hotkeys, desktop and plugins
- Settings menu items open the settings app on the right page
- Rebuild the main menu and fix some styling bugs
- Panel tells the window manager to keep windows off of it instead of setting margins in the window manager
- Partially fix panel resize bug
- Update the xfwm4 theme and add the Round-Light Forest theme
- Fix panel button text-only size bug
- Fix fade in bug
- Make the desktop drag box themeable
- Keep desktop icons within the screen's available geometry
- Elide long text on desktop icons and window list buttons
- Draw a dark glow behind desktop icon text instead of a second label
- Volume control listens on D-Bus so hotkeys can be mapped to it
- Quicklaunch uses XdgDesktopFile
- Add a file dialog to the desktop wallpaper settings
- Fix settings templates bug

* Thu Oct 21 2021 Nicholas Yoder <nicholasyoder@proton.me> - 0.6-1
- Panel moves when the screen resolution changes
- Improve main menu keyboard navigation
- Rebuild the desktop and add file management utilities
- Add polkit authentication agent
- Add create new file/folder to the desktop context menu
- Add desktop file rename
- Fix desktop wallpaper render bug
- Add a popup menu to the volume plugin
- Begin using XdgDesktopFile for desktop entries

* Wed Jul 14 2021 Nicholas Yoder <nicholasyoder@proton.me> - 0.5-1
- Port to Qt5 (rebuilt plugin by plugin) and from Xlib to XCB
- Rebuild the main menu, clock, volume, CPU/memory monitor, sensor, quicklaunch, desktop, window list, desktop switcher and Forest Settings
- Add popup menus and a system tray
- Add notifications and the forest-services plugin
- Add working hotkeys
- Rename the power manager to forest-logout and reimplement it
- Reimplement fade in and popup animations with a new popup class
- Add panel button appearance and icon size settings
- Window list icon-only and tab modes reimplemented
- Upgrade CPU and memory monitors and add CPU monitor settings
- Change the settings list to a custom widget
- Allow quicklaunch icons to be reordered by dragging
- Fix monitor plugins' long interval delay
- Fix desktop icon move bug
- Fix main menu search dropping the first letter
- Fix main menu power item failing after a reload
- Fix window list crash and various memory leaks

* Tue Jul 14 2020 Nicholas Yoder <nicholasyoder@proton.me> - 0.4-1
- Add panel icon size and font size settings
- Better icon rendering on quicklaunch
- Move desktop icons out of the way of the panel and improve drag box and icon placement
- Add orientation change support to the sensor plugin
- Show the file path in menu item properties
- Replace the add plugin widget with a file dialog
- Move all D-Bus services to org.forest
- Fix wallpaper saving to the old settings
- Fix desktop icon reload bug
- Fix clock size bugs

* Mon Apr 27 2020 Nicholas Yoder <nicholasyoder@proton.me> - 0.3-1
- Add clock plugin
- Add hotkey manager
- Add battery monitor and notification manager
- Rebuild the main menu with search, keyboard navigation, a hotkey and a context menu
- Add "Add launcher" to the quicklaunch menu
- Rebuild the show desktop plugin
- Fade the power manager in and out and fade in on startup
- Add new GTK and xfwm4 themes and switch to xfwm4 as the window manager
- Add sensor plugin settings and clock settings
- Reorganize settings under "Forest"
- Fix popup box bug in several plugins
- Fix power manager bugs

* Mon Mar 16 2020 Nicholas Yoder <nicholasyoder@proton.me> - 0.2-1
- Add multi-desktop support
- Make the panel a free-floating window
- Add "General" settings page
- Switch panel and theme settings to Designer UI forms
- Add panel popup settings
- Make desktop icons movable
- Add desktop icon multiselect, copy, paste, rename and delete
- Add "Add to desktop" to main menu item context menu
- Add quicklaunch plugin and sensor monitor plugin
- Improve popup animations and volume control, and unify popup boxes
- Fix power manager showing on the taskbar

* Wed Feb 05 2020 Nicholas Yoder <nicholasyoder@proton.me> - 0.1-1
- Initial release: panel and desktop built from plugins
- Add panel button highlight settings
- First installable .deb package
