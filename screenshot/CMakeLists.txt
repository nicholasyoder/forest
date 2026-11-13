add_executable(forest-screenshot
    main.cpp
    capturecore.cpp
    clipboard.cpp
    capturecore.h
    clipboard.h
)

target_link_libraries(forest-screenshot PRIVATE Qt6::Core Qt6::Gui Qt6::DBus Qt6::WaylandClient Qt6::WaylandClientPrivate toplevels)
forest_generate_wayland_protocol_client(forest-screenshot
    ${CMAKE_SOURCE_DIR}/protocol/wlr-data-control-unstable-v1.xml)
forest_link_flogger(forest-screenshot)

# Not installed yet: usr/bin/forest-screenshot (the grim script) is still the
# shipped one until the overlay pickers land (docs/screenshot-plan.md).
