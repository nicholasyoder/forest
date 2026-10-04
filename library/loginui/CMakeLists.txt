add_library(loginui STATIC
    loginui.cpp
    loginui.h
)

set_target_properties(loginui PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(loginui PUBLIC Qt6::Widgets)
target_include_directories(loginui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
forest_link_miscutills(loginui)

install(TARGETS loginui ARCHIVE DESTINATION lib)
