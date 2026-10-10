add_library(activation STATIC
    xdgactivation.cpp
    xdgactivation.h
)

set_target_properties(activation PROPERTIES POSITION_INDEPENDENT_CODE ON)

pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
qt6_generate_wayland_protocol_client_sources(activation FILES
    "${WAYLAND_PROTOCOLS_DATADIR}/staging/xdg-activation/xdg-activation-v1.xml")

# Private API (QWaylandWindow::wlSurface): rebuild on every Qt minor bump.
target_link_libraries(activation PUBLIC Qt6::Widgets Qt6::WaylandClient PRIVATE Qt6::WaylandClientPrivate)
target_include_directories(activation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS activation ARCHIVE DESTINATION lib)
