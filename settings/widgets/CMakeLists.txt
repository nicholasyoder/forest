add_library(settings-widgets STATIC
    colorbutton.cpp
    colorbutton.h
    listwidget.cpp
    listwidget.h
    settingsbinder.cpp
    settingsbinder.h
    settingsrow.cpp
    settingsrow.h
)

set_target_properties(settings-widgets PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(settings-widgets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(settings-widgets PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)
target_include_directories(settings-widgets PRIVATE ${CMAKE_SOURCE_DIR}/library/miscutills)
target_link_libraries(settings-widgets PRIVATE miscutills)
