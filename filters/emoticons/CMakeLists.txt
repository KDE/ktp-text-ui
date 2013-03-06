set (ktptextui_message_filter_emoticons_SRCS
     emoticon-filter.cpp
)

kde4_add_ui_files (ktptextui_message_filter_emoticons_SRCS
)

kde4_add_plugin (ktptextui_message_filter_emoticons
                 ${ktptextui_message_filter_emoticons_SRCS}
)

target_link_libraries (ktptextui_message_filter_emoticons
    ktpchat
    ${QT_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KEMOTICONS_LIBS}
    ${KTP_LIBRARIES}
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
