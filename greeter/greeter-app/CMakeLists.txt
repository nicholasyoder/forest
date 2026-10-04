add_executable(forest-greeter
    main.cpp
    greetdclient.cpp
    greeterwindow.cpp
    userlistmodel.cpp
    sessionlistmodel.cpp
    userselectview.cpp
    passwordview.cpp
    sessionselectview.cpp
    greetdclient.h
    greeterwindow.h
    userlistmodel.h
    sessionlistmodel.h
    userselectview.h
    passwordview.h
    sessionselectview.h
)


target_link_libraries(forest-greeter PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)
forest_link_flogger(forest-greeter)
forest_link_miscutills(forest-greeter)
forest_link_loginui(forest-greeter)

install(TARGETS forest-greeter RUNTIME DESTINATION bin COMPONENT greeter)
