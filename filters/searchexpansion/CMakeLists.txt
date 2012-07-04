set (ktptextui_message_filter_searchexpansion_SRCS
     searchexpansion-filter.cpp
)

kde4_add_ui_files (ktptextui_message_filter_searchexpansion_SRCS
)

kde4_add_plugin (ktptextui_message_filter_searchexpansion
                 ${ktptextui_message_filter_searchexpansion_SRCS}
)

target_link_libraries (ktptextui_message_filter_searchexpansion
    ktpchat
    ${QT_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KEMOTICONS_LIBS}
)

# Install:
install (TARGETS ktptextui_message_filter_searchexpansion
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES ktptextui_message_filter_searchexpansion.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

