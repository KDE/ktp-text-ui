add_library (ktptextui_message_filter_formatting MODULE format-filter.cpp)

target_link_libraries (ktptextui_message_filter_formatting
    KTp::CommonInternals
    KF5::CoreAddons
)

# Install:
install (TARGETS ktptextui_message_filter_formatting
         DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_formatting.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_formatting.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_formatting.desktop
         DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
