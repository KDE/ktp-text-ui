include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/logviewer)

set(kcm_ktp_logviewer_behavior_SRCS behavior-config.cpp)

set(kcm_ktp_logviewer_behavior_UI behavior-config.ui)

kde4_add_ui_files(kcm_ktp_logviewer_behavior_SRCS ${kcm_ktp_logviewer_behavior_UI})

kde4_add_plugin(kcm_ktp_logviewer_behavior ${kcm_ktp_logviewer_behavior_SRCS})

target_link_libraries(kcm_ktp_logviewer_behavior
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KCMUTILS_LIBS}
    ktpchat
)

install(TARGETS
        kcm_ktp_logviewer_behavior
    DESTINATION ${PLUGIN_INSTALL_DIR}
)
install(FILES
        kcm_ktp_logviewer_behavior.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
)
