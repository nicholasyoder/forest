add_executable(forest-session
    main.cpp
    sessionapp.cpp
    sessionapp.h
)

set_target_properties(forest-session PROPERTIES CXX_STANDARD 17)

target_link_libraries(forest-session PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6Xdg)
forest_link_flogger(forest-session)

install(TARGETS forest-session RUNTIME DESTINATION bin)
