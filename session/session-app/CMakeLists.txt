add_executable(forest-session
    main.cpp
    sessionapp.cpp
    sessionapp.h
    supervisedprocess.cpp
    supervisedprocess.h
)


target_link_libraries(forest-session PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus Qt6Xdg)
forest_link_flogger(forest-session)

install(TARGETS forest-session RUNTIME DESTINATION bin)
