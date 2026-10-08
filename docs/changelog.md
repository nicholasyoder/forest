
Changelog
============

* Unreleased
  - Add a Displays settings page: arrangement, resolution, refresh rate, scale, orientation and primary display, with a keep-or-revert prompt
  - Add display profiles, picked automatically when monitors are plugged in, with hotkeys to switch between them
  - Add a Built-in action list and a D-Bus argument field to hotkey settings
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

* Release 0.7.5 - 2022-05-26
  - Rebuilt mainmenu plugin

* Release 0.7.1 - 2022-06-11

* Release 0.7.0 - 2022-05-26

* Release 0.6 - 2021-10-21

* Release 0.5 - 2021-07-14

* Release 0.4 - 2020-07-13

* Release 0.3 - 2020-05-05

* Release 0.2 - 2020-03-13

* Release 0.1 - 2020-02-05
