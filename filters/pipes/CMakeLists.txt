set (ktptextui_message_filter_pipes_SRCS
     pipes-filter.cpp
     pipes-prefs.cpp
)

kde4_add_plugin (ktptextui_message_filter_pipes
                 ${ktptextui_message_filter_pipes_SRCS}
)

target_link_libraries (ktptextui_message_filter_pipes
    ktpchat
    ${QT_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KEMOTICONS_LIBS}
)

# Install:
install (TARGETS ktptextui_message_filter_pipes
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES ktptextui_message_filter_pipes.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

