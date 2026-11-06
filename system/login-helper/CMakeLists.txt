add_executable(forest-login-helper main.cpp)

# QImageReader's format plugins live in QtGui.
target_link_libraries(forest-login-helper PRIVATE Qt6::Core Qt6::Gui)

install(TARGETS forest-login-helper RUNTIME DESTINATION lib/forest)
