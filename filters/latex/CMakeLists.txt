set (ktptextui_message_filter_latex_SRCS
     latex-filter.cpp
)

kde4_add_kcfg_files(ktptextui_message_filter_latex_SRCS latexconfig.kcfgc)

kde4_add_plugin (ktptextui_message_filter_latex
                 ${ktptextui_message_filter_latex_SRCS}
)

target_link_libraries (ktptextui_message_filter_latex
    ktpchat
    ${QT_LIBRARIES}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KTP_LIBRARIES}
)

set (kcm_ktptextui_message_filter_latex_SRCS
     latex-config.cpp
)

kde4_add_kcfg_files(kcm_ktptextui_message_filter_latex_SRCS latexconfig.kcfgc)

kde4_add_ui_files (kcm_ktptextui_message_filter_latex_SRCS
    latex-config.ui
)

kde4_add_plugin (kcm_ktptextui_message_filter_latex
                 ${kcm_ktptextui_message_filter_latex_SRCS}
)

target_link_libraries (kcm_ktptextui_message_filter_latex
    ktpchat
    ${QT_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
)

# Install:
install (TARGETS ktptextui_message_filter_latex
                 kcm_ktptextui_message_filter_latex
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_latex.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_latex.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_latex.desktop
               ${CMAKE_CURRENT_SOURCE_DIR}/kcm_ktptextui_message_filter_latex.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)
