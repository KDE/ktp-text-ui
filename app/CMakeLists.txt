include_directories(${CMAKE_SOURCE_DIR}/lib)

set(ktp-text-ui_SRCS
        main.cpp
        telepathy-chat-ui.cpp
        chat-window.cpp
        chat-tab.cpp
        invite-contact-dialog.cpp
)

kde4_add_executable(ktp-text-ui ${ktp-text-ui_SRCS})

target_link_libraries(ktp-text-ui
            ${KDE4_KDECORE_LIBS}
            ${KDE4_KDEUI_LIBS}
            ${KDE4_KIO_LIBS}
            ${TELEPATHY_QT4_LIBRARIES}
            ${KTP_LIBRARIES}
            ${KTP_MODELS_LIBRARIES}
            ${KTP_WIDGETS_LIBRARIES}
            ${QT_QTWEBKIT_LIBRARY}
            ${KDE4_KNOTIFYCONFIG_LIBS}
            ${KDE4_KCMUTILS_LIBS}
            ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
            ktpchat)

configure_file(org.freedesktop.Telepathy.Client.KTp.TextUi.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.TextUi.service)

install(TARGETS ktp-text-ui DESTINATION ${LIBEXEC_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.TextUi.service
        DESTINATION ${DBUS_SERVICES_INSTALL_DIR})
install(FILES KTp.TextUi.client DESTINATION ${SHARE_INSTALL_PREFIX}/telepathy/clients/)
install(FILES chatwindow.rc
        DESTINATION  ${DATA_INSTALL_DIR}/ktp-text-ui/)
