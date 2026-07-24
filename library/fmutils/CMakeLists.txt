# fmutils is header-only (all methods are inline in fmutils.h), so it is
# declared as an INTERFACE library rather than a compiled SHARED/STATIC one.
add_library(fmutils INTERFACE)

target_include_directories(fmutils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fmutils INTERFACE Qt6::Core Qt6::Gui Qt6::Widgets)
