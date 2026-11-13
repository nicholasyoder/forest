add_executable(forest-screenshot
    main.cpp
    capturecore.cpp
    clipboard.cpp
    picker.cpp
    launcher.cpp
    capturecore.h
    clipboard.h
    picker.h
    launcher.h
)

target_link_libraries(forest-screenshot PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus Qt6::WaylandClient Qt6::WaylandClientPrivate toplevels)
forest_generate_wayland_protocol_client(forest-screenshot
    ${CMAKE_SOURCE_DIR}/protocol/wlr-data-control-unstable-v1.xml)
forest_link_flogger(forest-screenshot)
target_include_directories(forest-screenshot PRIVATE ${CMAKE_SOURCE_DIR}/library/fstyleloader)
forest_link_layershellqt(forest-screenshot)

install(TARGETS forest-screenshot DESTINATION bin)
