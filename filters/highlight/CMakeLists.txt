add_library (ktptextui_message_filter_highlight MODULE highlight-filter.cpp)

target_link_libraries (ktptextui_message_filter_highlight
    ktpchat
    
    KTp::CommonInternals
)

# Install:
install (TARGETS ktptextui_message_filter_highlight
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_highlight.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_highlight.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_highlight.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

