set (ktptextui_message_filter_escape_SRCS
     escape-filter.cpp
)

kde4_add_ui_files (ktptextui_message_filter_escape_SRCS
)

kde4_add_plugin (ktptextui_message_filter_escape
                 ${ktptextui_message_filter_escape_SRCS}
)

target_link_libraries (ktptextui_message_filter_escape
    ktpchat
    ${QT_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KEMOTICONS_LIBS}
)

# Install:
install (TARGETS ktptextui_message_filter_escape
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES ktptextui_message_filter_escape.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

