add_executable(forest-logout
    iconbutton.cpp
    main.cpp
    logout.cpp
    iconbutton.h
    logout.h
)

target_compile_definitions(forest-logout PRIVATE QT_DEPRECATED_WARNINGS)
set_target_properties(forest-logout PROPERTIES CXX_STANDARD 11)

target_link_libraries(forest-logout PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus)
forest_link_flogger(forest-logout)
forest_link_layershellqt(forest-logout)
forest_link_layeroverlay(forest-logout)

install(TARGETS forest-logout RUNTIME DESTINATION bin)
