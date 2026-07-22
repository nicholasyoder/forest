add_library(pluginutills STATIC
    pluginutills.cpp
    pluginutills.h
    app_plugin_interface.h
    settings_plugin_interface.h
)

set_target_properties(pluginutills PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(pluginutills PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(pluginutills PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)

install(TARGETS pluginutills ARCHIVE DESTINATION lib)
