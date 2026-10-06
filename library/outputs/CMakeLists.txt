add_library(outputs STATIC
    outputtypes.cpp
    outputmanager.cpp
    displayprofiles.cpp
    outputtypes.h
    outputmanager.h
    displayprofiles.h
)

set_target_properties(outputs PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(outputs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(outputs PUBLIC Qt6::Core Qt6::Gui Qt6::WaylandClient)

qt6_generate_wayland_protocol_client_sources(outputs FILES
    ${CMAKE_SOURCE_DIR}/protocol/wlr-output-management-unstable-v1.xml)
# PUBLIC so users including outputmanager.h find the generated qwayland-*.h.
target_include_directories(outputs PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS outputs ARCHIVE DESTINATION lib)
