add_library (ktptextui_message_filter_emoticons MODULE emoticon-filter.cpp)

target_link_libraries (ktptextui_message_filter_emoticons
    KF5::Emoticons
    KF5::CoreAddons
    KTp::CommonInternals
)

# Install:
install (TARGETS ktptextui_message_filter_emoticons
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_emoticons.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_emoticons.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_emoticons.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)
