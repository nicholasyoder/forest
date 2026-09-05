add_library(windowlist MODULE
    imagepopup.cpp
    settingswidget.cpp
    windowbutton.cpp
    windowlist.cpp
    foreigntoplevelmanager.cpp
    foreigntoplevelhandle.cpp
    extforeigntoplevellist.cpp
    extforeigntoplevelhandle.cpp
    iconresolver.cpp
    closebutton.h
    imagepopup.h
    settingswidget.h
    windowbutton.h
    windowlist.h
    foreigntoplevelmanager.h
    foreigntoplevelhandle.h
    extforeigntoplevellist.h
    extforeigntoplevelhandle.h
    iconresolver.h
    settingswidget.ui
)

target_compile_definitions(windowlist PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)
# QWaylandClientExtensionTemplate needs C++17 (if constexpr); the CXX_STANDARD 11
# pin below predates this target's Wayland client binding.
set_target_properties(windowlist PROPERTIES CXX_STANDARD 17)

target_include_directories(windowlist PRIVATE ${CMAKE_SOURCE_DIR}/library)

target_link_libraries(windowlist PRIVATE
    Qt6::Widgets Qt6::DBus
)
forest_link_panel_library(windowlist)
forest_link_miscutills(windowlist)
forest_generate_wayland_protocol_client(windowlist ${CMAKE_SOURCE_DIR}/protocol/wlr-foreign-toplevel-management-unstable-v1.xml)

# ext-foreign-toplevel-list-v1 is part of the wayland-protocols package's
# staging/ directory (unlike wlr-foreign-toplevel-management-unstable-v1.xml
# above, vendored since it isn't installed by any system package) - see
# panel-library/CMakeLists.txt's identical note for ext-workspace-v1.
pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
forest_generate_wayland_protocol_client(windowlist
    "${WAYLAND_PROTOCOLS_DATADIR}/staging/ext-foreign-toplevel-list/ext-foreign-toplevel-list-v1.xml")

install(TARGETS windowlist LIBRARY DESTINATION lib/forest/panel)
