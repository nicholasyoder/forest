add_library(settings-widgets STATIC
    listwidget.cpp
    listwidget.h
)

set_target_properties(settings-widgets PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(settings-widgets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(settings-widgets PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)
