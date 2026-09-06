# ForestDeps.cmake - Helper functions replacing the qmake .pri include pattern

# forest_link_miscutills(target)
# Adds include paths and links the miscutills static library
function(forest_link_miscutills target)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/library/miscutills)
    target_link_libraries(${target} PRIVATE miscutills)
endfunction()

# forest_link_pluginutills(target)
# Adds include paths and links the pluginutills static library
function(forest_link_pluginutills target)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/library/pluginutills)
    target_link_libraries(${target} PRIVATE pluginutills)
endfunction()

# forest_link_settings_widgets(target)
# Adds include paths and links the settings-widgets static library
function(forest_link_settings_widgets target)
    target_link_libraries(${target} PRIVATE settings-widgets)
endfunction()

# forest_link_panel_library(target)
# Adds include paths and links the panel-library static library
function(forest_link_panel_library target)
    target_link_libraries(${target} PRIVATE panel-library)
endfunction()

# forest_link_flogger(target)
# Adds include paths and links the flogger static library
function(forest_link_flogger target)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/library/flogger)
    target_link_libraries(${target} PRIVATE flogger)
endfunction()

# forest_link_layershellqt(target)
# Links the LayerShellQt (wlr-layer-shell-unstable-v1) interface library
function(forest_link_layershellqt target)
    target_link_libraries(${target} PRIVATE LayerShellQt::Interface)
endfunction()

# forest_link_layeroverlay(target)
# Adds include paths and links the layeroverlay static library
function(forest_link_layeroverlay target)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/library/layeroverlay)
    target_link_libraries(${target} PRIVATE layeroverlay)
endfunction()

# forest_link_dbusmenu(target)
# Links the dbusmenu-lxqt library (DBusMenu protocol client, used to render
# a StatusNotifierItem's context menu)
function(forest_link_dbusmenu target)
    target_link_libraries(${target} PRIVATE dbusmenu-lxqt)
endfunction()

# forest_generate_wayland_protocol_client(target xml_path)
# Generates and adds Qt Wayland client bindings for a vendored protocol XML
# (via qt6_generate_wayland_protocol_client_sources) and links Qt6::WaylandClient.
# `target` must already exist (add_library/add_executable called first).
function(forest_generate_wayland_protocol_client target xml_path)
    qt6_generate_wayland_protocol_client_sources(${target} FILES ${xml_path})
    target_link_libraries(${target} PRIVATE Qt6::WaylandClient)
endfunction()
