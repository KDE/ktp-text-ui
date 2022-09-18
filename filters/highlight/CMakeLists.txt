add_library (ktptextui_message_filter_highlight MODULE highlight-filter.cpp)

target_link_libraries (ktptextui_message_filter_highlight
    ktpchat
    KF5::CoreAddons
    KTp::CommonInternals
)

# Install:
install (TARGETS ktptextui_message_filter_highlight
         DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_highlight.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_highlight.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_highlight.desktop
         DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)

