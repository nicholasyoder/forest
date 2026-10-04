add_library(sessionlock STATIC
    sessionlock.cpp
    sessionlock.h
)

set_target_properties(sessionlock PROPERTIES POSITION_INDEPENDENT_CODE ON)

pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
qt6_generate_wayland_protocol_client_sources(sessionlock FILES
    "${WAYLAND_PROTOCOLS_DATADIR}/staging/ext-session-lock/ext-session-lock-v1.xml")

# Private API: rebuild on every Qt minor bump.
target_link_libraries(sessionlock PUBLIC Qt6::Gui PRIVATE Qt6::WaylandClient Qt6::WaylandClientPrivate)
target_include_directories(sessionlock PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS sessionlock ARCHIVE DESTINATION lib)
