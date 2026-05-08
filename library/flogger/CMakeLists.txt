add_library(flogger STATIC
    flogger.cpp
    flogger.h
)

target_include_directories(flogger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(flogger PRIVATE Qt6::Core)

install(TARGETS flogger ARCHIVE DESTINATION usr/lib)
