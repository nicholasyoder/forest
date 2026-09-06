add_executable(forest
    main.cpp
    forest.cpp
    settings_upgrade_manager.cpp
    forest.h
    forestxcbeventfilter.h
    settings_upgrade_manager.h
)

target_compile_definitions(forest PRIVATE QT_DEPRECATED_WARNINGS)
set_target_properties(forest PROPERTIES CXX_STANDARD 11)

target_link_libraries(forest PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus
)
forest_link_pluginutills(forest)
forest_link_flogger(forest)
forest_link_layershellqt(forest)
forest_link_layeroverlay(forest)

install(TARGETS forest RUNTIME DESTINATION bin)
