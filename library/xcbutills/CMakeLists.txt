add_library(xcbutills STATIC
    xcbutills.cpp
    xcbutills.h
    numlock.cpp
    numlock.h
)

set_target_properties(xcbutills PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(xcbutills PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
target_include_directories(xcbutills PUBLIC /usr/include/KF6/KWindowSystem)

install(TARGETS xcbutills ARCHIVE DESTINATION usr/lib)
