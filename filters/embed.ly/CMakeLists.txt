set (ktptextui_message_filter_embedly_SRCS
     embedly-filter.cpp
)

kde4_add_plugin (ktptextui_message_filter_embedly
                 ${ktptextui_message_filter_embedly_SRCS}
)

target_link_libraries (ktptextui_message_filter_embedly
    ktpchat
    ${QT_LIBRARIES}
    ${KDE4_KIO_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KTP_LIBRARIES}
    ${QJSON_LIBRARIES}
)

# Install:
install (TARGETS ktptextui_message_filter_embedly
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_embedly.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_embedly.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_embedly.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)
