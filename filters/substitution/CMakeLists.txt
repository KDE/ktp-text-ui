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

set (kcm_ktp_filter_config_substitution_SRCS
    substitution-config.cpp
    substitution-prefs.cpp
)

kde4_add_ui_files (kcm_ktp_filter_config_substitution_SRCS
    substitution-config.ui
)

kde4_add_plugin (kcm_ktp_filter_config_substitution
    ${kcm_ktp_filter_config_substitution_SRCS}
)

target_link_libraries (kcm_ktp_filter_config_substitution
    ktpchat
    ${QT_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
)

# Install:
install (TARGETS
            ktptextui_message_filter_substitution
            kcm_ktp_filter_config_substitution
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES
            ktptextui_message_filter_substitution.desktop
            kcm_ktp_filter_config_substitution.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)
