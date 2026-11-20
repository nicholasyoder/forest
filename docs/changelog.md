
Changelog
============

* Unreleased
  - Workspaces are matched to Biome's by their ext-workspace `id` instead of the order they arrive in (falling back to arrival order on compositors that send none); deskswitch and the windowlist share one workspace client class, and `forest-screenshot` no longer binds workspace clients
  - Rework notification popups: `expire_timeout` 0 now never expires (a default of 0 no longer closes popups instantly, and max is raised to min if set lower); a notification with `replaces_id` updates its popup in place instead of remapping it; `CloseNotification` on an unknown id returns an error; no more 10 ms repaint timer per popup (the timeout bar is animated and cached); at most 5 popups are shown, the rest wait for a free slot
  - Advertise `body-markup`: notification bodies render `<b>`, `<i>`, `<u>`, `<br>` and http/https/mailto links (clickable), with everything else escaped or dropped; summaries stay plain text
  - Applet fixes: the battery warning no longer fires while charging (and updates its own notification), and the battery applet hides when there is no battery; the CPU monitor no longer double-counts guest time (iowait still counts as busy) and the CPU, memory and temperature applets no longer spin on a 0 interval; the memory monitor reads `/proc/meminfo` by key; the temperature applet only reads the sensors it displays, shows `°C` correctly, handles no sensors and uses theme text colour; Quicklaunch keeps every file of a multi-file drop, decodes paths and no longer overwrites entries; the tray honours `ItemIsMenu` and accumulates fine scroll; mute toggles the playback switch rather than zeroing volume, and the volume wheel ignores horizontal scroll and fine deltas
  - Settings and displays fixes: changing a page in the background no longer jumps the settings window to it; the Cursor page no longer rewrites `index.theme` when opened or on every keystroke and only scans real icon directories; About reads `QSysInfo` and `/proc` instead of running shell commands, and the Displays page connects to the compositor only when first opened; the login wallpaper page is now "Login & Lock Wallpaper"; the Panel page keeps applets it can't load instead of erasing them (and duplicate separators no longer merge); the displays service re-applies the saved profile when a monitor reconnects off its layout; the primary-screen setting is shared between the shell, displays service and settings

* Release 0.11.1 - 2026-11-17
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

* Release 0.11.0 - 2026-11-13
  - Move theme loading (parent_themes chain, palette, per-layer CSS) into a header-only forest-theme library, installed with the new forest-theme-devel package so Biome can use it
  - Move menu styling into a per-theme shared.css, also loaded by Biome for its window menu; add arrow spacing for submenu items; call virtual desktops "workspaces" in the windowlist menu
  - Add a notification position setting (any screen corner), and stack simultaneous notifications instead of overlapping them, reflowing as they close
  - Hold notifications that arrive during login until the startup fade-in finishes, so they no longer draw over it
  - Theme the settings app from the selected desktop theme (light or dark), restyling live when the theme changes
  - Draw text box, spin box, checkbox and radio button outlines in a light color on dark themes
  - Replace the grim and slurp screenshot script with a native forest-screenshot: a capture dialog in the main menu (whole workspace, single monitor, active window or region, with a delay), plus --full, --monitor, --window and --region flags for hotkeys. A bare forest-screenshot now opens the dialog instead of capturing the whole screen; bind --full for that
  - Show live window previews in the windowlist hover popup, including for minimized and other-workspace windows (needs Biome with per-window capture; otherwise the app icon is shown), with a preview size setting
  - Drag a multi-icon selection on the desktop as a group, keeping the icons' relative positions

* Release 0.10.1 - 2026-11-11
  - Port to Fedora and Qt 6.11; no functional changes
  - Plugins install to the distro libdir (/usr/lib64/forest on Fedora); stored panel applet paths are migrated

* Release 0.10.0 - 2026-11-09
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

* Release 0.9.0 - 2026-10-05
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

* Release 0.8.0 - 2026-07-21
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

* Release 0.7.9 - 2025-03-25
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

* Release 0.7.8 - 2024-09-23
  - Fix panel on top of fullscreen windows
  - Fix library linking
  - Add docs directory with changelog and development notes
  - Add auto-hide functionality to panel
  - Add session manager
  - Rebuild the settings app
  - Restructure project
  - Improve notifications
  - Add support for Meta single key global hotkey

* Release 0.7.7 - 2023-01-30
  - Start using KWindowSystem for some things
  - Allow window reordering on the windowlist
  - Add SNI systemtray but keep the old one available as well

* Release 0.7.6 - 2022-12-17
  - Fixed mainmenu name
  - Added hotkeys for screen brightness and screenshot
  - Enabled seperator between menu and quicklaunch

* Release 0.7.5 - 2022-12-17
  - (Changelog entries for the point releases between 0.7.0 and 0.7.5 were lost)
  - Start tracking the project in git and keeping a changelog in the project itself
  - Rebuilt mainmenu plugin
  - Fix faded pixel at the top of the CPU and RAM monitor bars

* Release 0.7.0 - 2022-05-26
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

* Release 0.6 - 2021-10-21
  - Panel moves when the screen resolution changes
  - Improve main menu keyboard navigation
  - Rebuild the desktop and add file management utilities
  - Add polkit authentication agent
  - Add create new file/folder to the desktop context menu
  - Add desktop file rename
  - Fix desktop wallpaper render bug
  - Add a popup menu to the volume plugin
  - Begin using XdgDesktopFile for desktop entries

* Release 0.5 - 2021-07-14
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

* Release 0.4 - 2020-07-14
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

* Release 0.3 - 2020-04-27
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

* Release 0.2 - 2020-03-16
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

* Release 0.1 - 2020-02-05
  - Initial release: panel and desktop built from plugins
  - Add panel button highlight settings
  - First installable .deb package
