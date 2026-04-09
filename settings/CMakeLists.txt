add_subdirectory(widgets)

add_executable(forest-settings
    breadcrumbwidget.cpp
    catlistwidget.cpp
    main.cpp
    settingsmanager.cpp
    breadcrumbwidget.h
    catlistwidget.h
    settingsmanager.h
    widgets/listwidget.h
)

set_target_properties(forest-settings PROPERTIES CXX_STANDARD 17)

target_link_libraries(forest-settings PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
forest_link_pluginutills(forest-settings)

install(TARGETS forest-settings RUNTIME DESTINATION usr/bin)
