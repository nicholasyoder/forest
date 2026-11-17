# Header-only theme format loader, installed as ForestTheme::ForestTheme so
# Biome can build against it (find_package(ForestTheme)).
add_library(forest-theme INTERFACE)
add_library(ForestTheme::ForestTheme ALIAS forest-theme)
set_target_properties(forest-theme PROPERTIES EXPORT_NAME ForestTheme)

target_include_directories(forest-theme INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(forest-theme INTERFACE Qt6::Core Qt6::Gui)

install(FILES include/forest-theme/foresttheme.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/forest-theme)
install(TARGETS forest-theme EXPORT ForestThemeTargets)
install(EXPORT ForestThemeTargets NAMESPACE ForestTheme:: FILE ForestThemeTargets.cmake
        DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/ForestTheme)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/ForestThemeConfigVersion.cmake
    VERSION ${PROJECT_VERSION} COMPATIBILITY AnyNewerVersion ARCH_INDEPENDENT)
install(FILES ForestThemeConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/ForestThemeConfigVersion.cmake DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/ForestTheme)
