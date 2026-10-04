add_executable(forest-lockscreen
    main.cpp
    lockscreen.cpp
    lockwindow.cpp
    passwordcard.cpp
    pamauth.cpp
    lockscreen.h
    lockwindow.h
    passwordcard.h
    pamauth.h
)

# Not NDEBUG: Debian builds with CMAKE_BUILD_TYPE=None, which would keep the hatch.
target_compile_definitions(forest-lockscreen PRIVATE $<$<CONFIG:Debug>:FOREST_LOCKSCREEN_DEBUG>)

target_include_directories(forest-lockscreen PRIVATE ${CMAKE_SOURCE_DIR}/library/fstyleloader ${PAM_INCLUDE_DIR})
target_link_libraries(forest-lockscreen PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets ${PAM_LIBRARY} sessionlock)
forest_link_flogger(forest-lockscreen)
forest_link_miscutills(forest-lockscreen)
forest_link_loginui(forest-lockscreen)

install(TARGETS forest-lockscreen RUNTIME DESTINATION bin)
