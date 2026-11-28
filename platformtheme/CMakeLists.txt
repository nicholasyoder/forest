find_package(qt6xdgiconloader REQUIRED)

add_library(forestplatformtheme MODULE
    main.cpp
    forestplatformtheme.cpp
    forestplatformtheme.h
)

target_compile_definitions(forestplatformtheme PRIVATE QT_DEPRECATED_WARNINGS)
target_link_libraries(forestplatformtheme PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::GuiPrivate
    Qt6XdgIconLoader
    ForestTheme::ForestTheme
)

if(QT6_INSTALL_PLUGINS)
    set(FOREST_QT6_PLATFORMTHEMES_DIR "${QT6_INSTALL_PLUGINS}/platformthemes")
else()
    set(FOREST_QT6_PLATFORMTHEMES_DIR "${CMAKE_INSTALL_LIBDIR}/qt6/plugins/platformthemes")
endif()

install(TARGETS forestplatformtheme LIBRARY DESTINATION "${FOREST_QT6_PLATFORMTHEMES_DIR}")
