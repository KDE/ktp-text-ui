add_library (ktptextui_message_filter_otr MODULE otr-filter.cpp)

target_link_libraries (ktptextui_message_filter_otr
    KF5::KIOWidgets
    KF5::I18n
    KTp::CommonInternals
    KTp::OTR
)

# Install:
install (TARGETS ktptextui_message_filter_otr
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_otr.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_otr.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_otr.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)
