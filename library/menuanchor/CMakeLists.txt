add_library(menuanchor STATIC
    menuanchor.cpp
    menuanchor.h
)

set_target_properties(menuanchor PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(menuanchor PUBLIC Qt6::Widgets)
target_include_directories(menuanchor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS menuanchor ARCHIVE DESTINATION lib)
