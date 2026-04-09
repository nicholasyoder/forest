add_executable(forest-logout
    iconbutton.cpp
    imagewidget.cpp
    main.cpp
    logout.cpp
    iconbutton.h
    imagewidget.h
    logout.h
)

target_compile_definitions(forest-logout PRIVATE QT_DEPRECATED_WARNINGS)
set_target_properties(forest-logout PROPERTIES CXX_STANDARD 11)

target_link_libraries(forest-logout PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus)

install(TARGETS forest-logout RUNTIME DESTINATION usr/bin)
