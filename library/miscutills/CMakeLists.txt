add_library(miscutills STATIC
    miscutills.cpp
    miscutills.h
)

set_target_properties(miscutills PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(miscutills PRIVATE Qt6::Core Qt6::DBus Qt6::Gui)

install(TARGETS miscutills ARCHIVE DESTINATION lib)
