cmake_minimum_required(VERSION 3.16)
project(forest VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Custom cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(ForestDeps)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus Network)

# Find external dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(KF6WINDOWSYSTEM IMPORTED_TARGET libKF6WindowSystem)
pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)
pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb xcb-composite xcb-image xcb-damage xcb-shape)
pkg_check_modules(XRENDER REQUIRED IMPORTED_TARGET xrender)
pkg_check_modules(XFIXES REQUIRED IMPORTED_TARGET xfixes)
pkg_check_modules(XDAMAGE REQUIRED IMPORTED_TARGET xdamage)
pkg_check_modules(XCURSOR REQUIRED IMPORTED_TARGET xcursor)
pkg_check_modules(ALSA REQUIRED IMPORTED_TARGET alsa)
#pkg_check_modules(SENSORS REQUIRED IMPORTED_TARGET libsensors)
pkg_check_modules(POLKIT_QT6 REQUIRED IMPORTED_TARGET polkit-qt6-agent-1)
pkg_check_modules(QT6XDG REQUIRED IMPORTED_TARGET Qt6Xdg)

find_package(KF6WindowSystem REQUIRED)
find_package(LayerShellQt REQUIRED)

find_path(SENSORS_INCLUDE_DIR sensors/sensors.h)
find_library(SENSORS_LIBRARY NAMES sensors)

if(NOT SENSORS_INCLUDE_DIR OR NOT SENSORS_LIBRARY)
    message(FATAL_ERROR "libsensors not found. Install libsensors-dev.")
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

# Install config and data files
install(DIRECTORY etc/forest/ DESTINATION /etc/forest PATTERN "greetd.toml" EXCLUDE)
install(FILES etc/forest/greetd.toml DESTINATION /etc/forest COMPONENT greeter)
install(DIRECTORY usr/share/xsessions/ DESTINATION /usr/share/xsessions)
install(DIRECTORY usr/share/applications/ DESTINATION /usr/share/applications)
install(DIRECTORY usr/share/forest/ DESTINATION /usr/share/forest USE_SOURCE_PERMISSIONS)
install(DIRECTORY usr/share/wallpapers/forest/ DESTINATION /usr/share/wallpapers/forest)
install(DIRECTORY usr/share/themes/ DESTINATION /usr/share/themes)
install(DIRECTORY usr/lib/systemd/system/ DESTINATION /usr/lib/systemd/system COMPONENT greeter)
