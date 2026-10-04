cmake_minimum_required(VERSION 3.16)
project(forest VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Custom cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(ForestDeps)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus Network WaylandClient)

# Find external dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(XCURSOR REQUIRED IMPORTED_TARGET xcursor)
pkg_check_modules(ALSA REQUIRED IMPORTED_TARGET alsa)
#pkg_check_modules(SENSORS REQUIRED IMPORTED_TARGET libsensors)
pkg_check_modules(POLKIT_QT6 REQUIRED IMPORTED_TARGET polkit-qt6-agent-1)
pkg_check_modules(QT6XDG REQUIRED IMPORTED_TARGET Qt6Xdg)
pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)

find_package(LayerShellQt REQUIRED)
find_package(dbusmenu-lxqt REQUIRED)

find_path(SENSORS_INCLUDE_DIR sensors/sensors.h)
find_library(SENSORS_LIBRARY NAMES sensors)

if(NOT SENSORS_INCLUDE_DIR OR NOT SENSORS_LIBRARY)
    message(FATAL_ERROR "libsensors not found. Install libsensors-dev.")
endif()

# Not pkg-config: pam.pc drags in libaudit-dev via Requires.private.
find_path(PAM_INCLUDE_DIR security/pam_appl.h)
find_library(PAM_LIBRARY NAMES pam)

if(NOT PAM_INCLUDE_DIR OR NOT PAM_LIBRARY)
    message(FATAL_ERROR "libpam not found. Install libpam0g-dev.")
endif()

# Build library subdirectories first (static libs)
add_subdirectory(library)

# Build applications and plugins
add_subdirectory(forest)
add_subdirectory(desktop)
add_subdirectory(panel)
add_subdirectory(services)
add_subdirectory(session)
add_subdirectory(settings)
add_subdirectory(logout)
add_subdirectory(system)
add_subdirectory(greeter)
add_subdirectory(locker)

# Install config and data files
install(PROGRAMS usr/bin/forest-screenshot DESTINATION bin)
install(DIRECTORY usr/share/wayland-sessions/ DESTINATION share/wayland-sessions)
install(DIRECTORY usr/share/applications/ DESTINATION share/applications)
install(DIRECTORY usr/share/forest/ DESTINATION share/forest USE_SOURCE_PERMISSIONS)
install(DIRECTORY usr/share/wallpapers/forest/ DESTINATION share/wallpapers/forest)
install(DIRECTORY usr/share/themes/ DESTINATION share/themes)
install(DIRECTORY usr/lib/systemd/system/ DESTINATION lib/systemd/system COMPONENT greeter)
# Last: non-root staging installs abort here, skipping any rule after.
install(DIRECTORY etc/forest/ DESTINATION /etc/forest PATTERN "greetd.toml" EXCLUDE)
install(FILES etc/forest/greetd.toml DESTINATION /etc/forest COMPONENT greeter)
install(FILES etc/pam.d/forest-locker DESTINATION /etc/pam.d)
install(FILES etc/biome/conf.d/50-forest.conf DESTINATION /etc/biome/conf.d)
