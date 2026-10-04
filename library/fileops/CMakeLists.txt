add_library(fileops STATIC
    filejob.cpp
    filejob.h
    fileops.cpp
    fileops.h
)

set_target_properties(fileops PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(fileops PUBLIC Qt6::Widgets)
target_include_directories(fileops PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS fileops ARCHIVE DESTINATION lib)
