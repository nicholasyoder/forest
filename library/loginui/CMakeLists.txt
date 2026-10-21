add_library(loginui STATIC
    loginui.cpp
    loginui.h
    passwordform.cpp
    passwordform.h
    screenset.cpp
    screenset.h
)

set_target_properties(loginui PROPERTIES POSITION_INDEPENDENT_CODE ON)

# WaylandClientPrivate for PasswordForm's Caps Lock state.
target_link_libraries(loginui PUBLIC Qt6::Widgets PRIVATE Qt6::WaylandClientPrivate)
target_include_directories(loginui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
forest_link_miscutills(loginui)

install(TARGETS loginui ARCHIVE DESTINATION lib)
