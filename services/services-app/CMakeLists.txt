add_library(services-app MODULE
    services.cpp
    hotkeys/foresthotkeys.cpp
    hotkeys/hotkey.cpp
    hotkeys/globalshortcutsportal.cpp
    notifications/notify.cpp
    notifications/notifyadapter.cpp
    notifications/notifypopup.cpp
    polkit/polkitagent.cpp
    polkit/polkitdialog.cpp
    showdesktop/showdesktop.cpp
    systemtray/statusnotifierwatcher.cpp
    displays/displays.cpp
    displays/confirmcards.cpp
    services.h
    hotkeys/foresthotkeys.h
    hotkeys/hotkey.h
    hotkeys/globalshortcutsportal.h
    hotkeys/keysym_table.h
    notifications/notify.h
    notifications/notifyadapter.h
    notifications/notifypopup.h
    polkit/polkitagent.h
    polkit/polkitdialog.h
    polkit/polkitdialog.ui
    showdesktop/showdesktop.h
    systemtray/statusnotifierwatcher.h
    displays/displays.h
    displays/confirmcards.h
)

target_compile_definitions(services-app PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS FOREST_VERSION="${PROJECT_VERSION}")

target_include_directories(services-app PRIVATE
    ${CMAKE_SOURCE_DIR}/library
)

target_link_libraries(services-app PRIVATE
    Qt6::Widgets Qt6::DBus
    PkgConfig::POLKIT_QT6
    PkgConfig::XKBCOMMON
    Qt6Xdg
)

forest_link_miscutills(services-app)
forest_link_layershellqt(services-app)
forest_link_toplevels(services-app)
forest_link_outputs(services-app)
forest_link_hotkeyconfig(services-app)

install(TARGETS services-app LIBRARY DESTINATION lib/forest)
