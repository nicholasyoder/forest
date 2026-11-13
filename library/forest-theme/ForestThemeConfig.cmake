include(CMakeFindDependencyMacro)
find_dependency(Qt6 COMPONENTS Core Gui)
include("${CMAKE_CURRENT_LIST_DIR}/ForestThemeTargets.cmake")
