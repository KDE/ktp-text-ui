set (ktptextui_message_filter_substitution_SRCS
     substitution-prefs.cpp
     substitution-filter.cpp
)

kde4_add_plugin (ktptextui_message_filter_substitution
                 ${ktptextui_message_filter_substitution_SRCS}
)

target_link_libraries (ktptextui_message_filter_substitution
    ktpchat
    ${QT_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KEMOTICONS_LIBS}
)

# Install:
install (TARGETS ktptextui_message_filter_substitution
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES ktptextui_message_filter_substitution.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

