add_library(hotkeyconfig STATIC
    hotkeyconfig.cpp
    hotkeyconfig.h
)

set_target_properties(hotkeyconfig PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(hotkeyconfig PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(hotkeyconfig PUBLIC Qt6::Core)

install(TARGETS hotkeyconfig ARCHIVE DESTINATION lib)
