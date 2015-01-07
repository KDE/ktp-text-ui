add_library (ktptextui_message_filter_bugzilla MODULE bugzilla-filter.cpp)

target_link_libraries (ktptextui_message_filter_bugzilla
    KF5::KIOWidgets
    KTp::CommonInternals
)

# Install:
install (TARGETS ktptextui_message_filter_bugzilla
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_bugzilla.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_bugzilla.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_bugzilla.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

install (FILES showBugzillaInfo.js
         DESTINATION ${DATA_INSTALL_DIR}/ktelepathy)
