set (ktptextui_message_filter_bugzilla_SRCS
     bugzilla-filter.cpp
)

kde4_add_ui_files (ktptextui_message_filter_bugzilla_SRCS
)

kde4_add_plugin (ktptextui_message_filter_bugzilla
                 ${ktptextui_message_filter_bugzilla_SRCS}
)

target_link_libraries (ktptextui_message_filter_bugzilla
    ktpchat
    ${QT_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KEMOTICONS_LIBS}
)

# Install:
install (TARGETS ktptextui_message_filter_bugzilla
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES ktptextui_message_filter_bugzilla.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)
