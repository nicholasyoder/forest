add_library(fmutils SHARED
    fmutils.cpp
    fmutils.h
    fmutils_global.h
)

target_compile_definitions(fmutils PRIVATE QT_DEPRECATED_WARNINGS)

target_link_libraries(fmutils PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

install(TARGETS fmutils LIBRARY DESTINATION usr/lib)
