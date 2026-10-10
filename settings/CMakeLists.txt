add_subdirectory(widgets)

add_executable(forest-settings
    main.cpp
    settingsmanager.cpp
    settingsmanager.h
)


target_link_libraries(forest-settings PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus)
forest_link_pluginutills(forest-settings)
forest_link_flogger(forest-settings)
forest_link_activation(forest-settings)
forest_link_settings_widgets(forest-settings)

install(TARGETS forest-settings RUNTIME DESTINATION bin)
