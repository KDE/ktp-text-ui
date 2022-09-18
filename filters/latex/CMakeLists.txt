kconfig_add_kcfg_files(ktptextui_message_filter_latex_SRCS latexconfig.kcfgc)

add_library(ktptextui_message_filter_latex MODULE latex-filter.cpp
                 ${ktptextui_message_filter_latex_SRCS}
)

target_link_libraries (ktptextui_message_filter_latex
    ktpchat
    Qt5::Gui
    KTp::CommonInternals
    KF5::CoreAddons
    KF5::KCMUtils
)

kconfig_add_kcfg_files(kcm_ktptextui_message_filter_latex_SRCS latexconfig.kcfgc)

ki18n_wrap_ui(kcm_ktptextui_message_filter_latex_SRCS
    latex-config.ui
)

add_library(kcm_ktptextui_message_filter_latex MODULE latex-config.cpp
                 ${kcm_ktptextui_message_filter_latex_SRCS}
)

target_link_libraries (kcm_ktptextui_message_filter_latex
    ktpchat
)

# Install:
install (TARGETS ktptextui_message_filter_latex
                 kcm_ktptextui_message_filter_latex
         DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_latex.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_latex.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_latex.desktop
               ${CMAKE_CURRENT_SOURCE_DIR}/kcm_ktptextui_message_filter_latex.desktop
         DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
