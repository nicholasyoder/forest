add_library(wallpaper STATIC
    fadeoverlay.cpp
    fadeoverlay.h
    wallpaper.cpp
    wallpaper.h
    wallpaperconfig.h
)

set_target_properties(wallpaper PROPERTIES POSITION_INDEPENDENT_CODE ON)

pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
qt6_generate_wayland_protocol_client_sources(wallpaper FILES
    "${WAYLAND_PROTOCOLS_DATADIR}/stable/viewporter/viewporter.xml"
    "${WAYLAND_PROTOCOLS_DATADIR}/staging/alpha-modifier/alpha-modifier-v1.xml")

# WaylandClientPrivate for QWaylandWindow::wlSurface() and QWaylandDisplay::shm().
target_include_directories(wallpaper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(wallpaper PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets
    PRIVATE Qt6::WaylandClient Qt6::WaylandClientPrivate)
