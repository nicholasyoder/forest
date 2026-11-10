
Changelog
============

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
