set (ktptextui_message_filter_youtube_SRCS
     youtube-filter.cpp
)

kde4_add_plugin (ktptextui_message_filter_youtube
                 ${ktptextui_message_filter_youtube_SRCS}
)

target_link_libraries (ktptextui_message_filter_youtube
    ktpchat
    ${QT_LIBRARIES}
    ${KDE4_KIO_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KTP_LIBRARIES}
)

# Install:
install (TARGETS ktptextui_message_filter_youtube
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_youtube.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_youtube.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_youtube.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)
