set (ktptextui_message_filter_pipes_SRCS
     pipes-prefs.cpp
     pipes-filter.cpp
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

set (kcm_ktp_filter_config_pipes_SRCS
    pipes-delegate.cpp
    pipes-model.cpp
    pipes-config.cpp
    pipes-prefs.cpp
)

kde4_add_ui_files (kcm_ktp_filter_config_pipes_SRCS
    pipes-config.ui
)

kde4_add_plugin (kcm_ktp_filter_config_pipes
    ${kcm_ktp_filter_config_pipes_SRCS}
)

target_link_libraries (kcm_ktp_filter_config_pipes
    ktpchat
    ${QT_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
)

# Install:
install (TARGETS
            ktptextui_message_filter_pipes
            kcm_ktp_filter_config_pipes
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES
            ktptextui_message_filter_pipes.desktop
            kcm_ktp_filter_config_pipes.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)


set (ktptextui_message_filter_pipes_tests_SRCS
    pipes-prefs.cpp
    pipes-prefstest.cpp
)

kde4_add_unit_test(ktptextui_message_filter_pipes_tests
    TESTNAME pipes-test
    ${ktptextui_message_filter_pipes_tests_SRCS}
)

target_link_libraries(ktptextui_message_filter_pipes_tests
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    ${TELEPATHY_QT4_LIBRARIES}
)