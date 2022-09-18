add_library (ktptextui_message_filter_bugzilla MODULE bugzilla-filter.cpp)

target_link_libraries (ktptextui_message_filter_bugzilla
    KF5::KIOWidgets
    KTp::CommonInternals
)

# Install:
install (TARGETS ktptextui_message_filter_bugzilla
         DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_bugzilla.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_bugzilla.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_bugzilla.desktop
         DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)

install (FILES showBugzillaInfo.js
         DESTINATION ${KDE_INSTALL_DATADIR}/ktelepathy)
