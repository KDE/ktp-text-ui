add_library (ktptextui_message_filter_formatting MODULE format-filter.cpp)

target_link_libraries (ktptextui_message_filter_formatting
    KTp::CommonInternals
)

# Install:
install (TARGETS ktptextui_message_filter_formatting
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_formatting.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_formatting.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_formatting.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)
