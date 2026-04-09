add_library(fstyleloader STATIC
    fstyleloader.cpp
    fstyleloader.h
)

target_link_libraries(fstyleloader PRIVATE Qt6::Core)

install(TARGETS fstyleloader ARCHIVE DESTINATION usr/lib)
