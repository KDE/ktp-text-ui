add_library (ktptextui_message_filter_tts MODULE tts-filter.cpp)

target_link_libraries (ktptextui_message_filter_tts
    KF5::CoreAddons
    KF5::I18n
    KTp::CommonInternals
    Qt5::TextToSpeech
)

# Install:
install (TARGETS ktptextui_message_filter_tts
         DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_tts.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_tts.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_tts.desktop
         DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
