add_library(lockerconfig STATIC
    lockerconfig.cpp
    lockerconfig.h
)

# PIC: also linked into the locker-settings plugin module.
set_target_properties(lockerconfig PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(lockerconfig PUBLIC Qt6::Core)
target_include_directories(lockerconfig PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
