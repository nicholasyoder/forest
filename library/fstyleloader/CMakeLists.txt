# fstyleloader is header-only (implementation lives in fstyleloader.h), so it
# is declared as an INTERFACE library rather than a compiled STATIC one.
add_library(fstyleloader INTERFACE)

target_include_directories(fstyleloader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fstyleloader INTERFACE Qt6::Core)
