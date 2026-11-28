add_executable(forest-login-helper main.cpp)

target_include_directories(forest-login-helper PRIVATE ${CMAKE_SOURCE_DIR}/library/wallpaper)
# QImageReader's format plugins live in QtGui.
target_link_libraries(forest-login-helper PRIVATE Qt6::Core Qt6::Gui)

install(TARGETS forest-login-helper RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/forest)
