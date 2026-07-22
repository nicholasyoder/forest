add_library(services-app MODULE
    services.cpp
    hotkeys/foresthotkeys.cpp
    hotkeys/hotkey.cpp
    hotkeys/qxt/qxtglobalshortcut.cpp
    hotkeys/qxt/qxtglobalshortcut_x11.cpp
    notifications/notify.cpp
    notifications/notifyadapter.cpp
    notifications/notifypopup.cpp
    polkit/polkitagent.cpp
    polkit/polkitdialog.cpp
    services.h
    hotkeys/foresthotkeys.h
    hotkeys/hotkey.h
    hotkeys/qxt/keymapper_x11.h
    hotkeys/qxt/qxtglobal.h
    hotkeys/qxt/qxtglobalshortcut.h
    hotkeys/qxt/qxtglobalshortcut_p.h
    notifications/notify.h
    notifications/notifyadapter.h
    notifications/notifypopup.h
    polkit/polkitagent.h
    polkit/polkitdialog.h
    polkit/polkitdialog.ui
)

target_compile_definitions(services-app PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)
set_target_properties(services-app PROPERTIES CXX_STANDARD 11)

target_include_directories(services-app PRIVATE
    ${CMAKE_SOURCE_DIR}/library
)

target_link_libraries(services-app PRIVATE
    Qt6::Widgets Qt6::DBus
    KF6WindowSystem
    PkgConfig::POLKIT_QT6
    Qt6Xdg
)
forest_link_xcbutills(services-app)

install(TARGETS services-app LIBRARY DESTINATION lib/forest)
