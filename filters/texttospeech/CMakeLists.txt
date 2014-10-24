qt5_add_dbus_interfaces(ktptextui_message_filter_tts_SRCS ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KSpeech.xml)

add_library (ktptextui_message_filter_tts MODULE tts-filter.cpp)

target_link_libraries (ktptextui_message_filter_tts
    KTp::CommonInternals
)

# Install:
install (TARGETS ktptextui_message_filter_tts
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_tts.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_tts.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_tts.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)
