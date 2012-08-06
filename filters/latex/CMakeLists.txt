set (ktptextui_message_filter_latex_SRCS
     latex-filter.cpp
)

kde4_add_plugin (ktptextui_message_filter_latex
                 ${ktptextui_message_filter_latex_SRCS}
)

target_link_libraries (ktptextui_message_filter_latex
    ktpchat
    ${QT_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KEMOTICONS_LIBS}
)

# Install:
install (TARGETS ktptextui_message_filter_latex
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES ktptextui_message_filter_latex.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

install (PROGRAMS ktp_message_filter_latex_converter.sh
         DESTINATION ${LIBEXEC_INSTALL_DIR}
)