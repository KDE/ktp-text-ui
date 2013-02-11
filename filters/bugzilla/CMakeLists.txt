find_package(QJSON REQUIRED)

set (ktptextui_message_filter_bugzilla_SRCS
     bugzilla-filter.cpp
)

kde4_add_plugin (ktptextui_message_filter_bugzilla
                 ${ktptextui_message_filter_bugzilla_SRCS}
)

target_link_libraries (ktptextui_message_filter_bugzilla
    ${QT_LIBRARIES}
    ${qjson_LIBRARIES}
    ${KDE4_KIO_LIBS}
    ${KTP_LIBRARIES}
    ${TELEPATHY_QT4_LIBRARIES}
)

# Install:
install (TARGETS ktptextui_message_filter_bugzilla
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES ktptextui_message_filter_bugzilla.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)
