add_library(layeroverlay STATIC
    layeroverlay.cpp
    layeroverlay.h
)

set_target_properties(layeroverlay PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(layeroverlay PUBLIC Qt6::Widgets LayerShellQt::Interface)
target_include_directories(layeroverlay PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS layeroverlay ARCHIVE DESTINATION lib)
