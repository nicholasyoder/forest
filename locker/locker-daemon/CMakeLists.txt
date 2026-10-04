add_executable(forest-locker
    main.cpp
    locker.cpp
    lockersettings.cpp
    idlewatcher.cpp
    displaypower.cpp
    locksupervisor.cpp
    logind.cpp
    screensaver.cpp
    locker.h
    lockersettings.h
    idlewatcher.h
    displaypower.h
    locksupervisor.h
    logind.h
    screensaver.h
)

pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
forest_generate_wayland_protocol_client(forest-locker
    "${WAYLAND_PROTOCOLS_DATADIR}/staging/ext-idle-notify/ext-idle-notify-v1.xml")
forest_generate_wayland_protocol_client(forest-locker
    ${CMAKE_SOURCE_DIR}/protocol/wlr-output-power-management-unstable-v1.xml)

# WaylandClientPrivate for QWaylandScreen::output() (public only from Qt 6.9).
target_link_libraries(forest-locker PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus Qt6::WaylandClientPrivate)
forest_link_flogger(forest-locker)
forest_link_layeroverlay(forest-locker)

install(TARGETS forest-locker RUNTIME DESTINATION bin)
