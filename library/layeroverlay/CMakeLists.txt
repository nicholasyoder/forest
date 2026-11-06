add_library(layeroverlay STATIC
    layeroverlay.cpp
    layeroverlay.h
)

set_target_properties(layeroverlay PROPERTIES POSITION_INDEPENDENT_CODE ON)

pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
# xdg-shell: layer-shell's get_popup references xdg_popup_interface.
qt6_generate_wayland_protocol_client_sources(layeroverlay FILES
    "${WAYLAND_PROTOCOLS_DATADIR}/stable/xdg-shell/xdg-shell.xml"
    "${WAYLAND_PROTOCOLS_DATADIR}/stable/viewporter/viewporter.xml"
    "${WAYLAND_PROTOCOLS_DATADIR}/staging/single-pixel-buffer/single-pixel-buffer-v1.xml"
    ${CMAKE_SOURCE_DIR}/protocol/wlr-layer-shell-unstable-v1.xml)

# single-pixel-buffer's generated code uses wl_buffer_interface without including the core protocol header.
target_compile_options(layeroverlay PRIVATE -include wayland-client-protocol.h)

# WaylandClientPrivate for QWaylandScreen::output() (public only from Qt 6.9).
target_link_libraries(layeroverlay PUBLIC Qt6::Gui PRIVATE Qt6::WaylandClient Qt6::WaylandClientPrivate)
target_include_directories(layeroverlay PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS layeroverlay ARCHIVE DESTINATION lib)
