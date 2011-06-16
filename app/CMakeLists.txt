include_directories(${TELEPATHY_KDE_TEXT_UI_SOURCE_DIR}/lib)

set(telepathy_kde_text_ui_SRCS
        main.cpp
        telepathy-chat-ui.cpp
        chat-window.cpp
        chat-tab.cpp
)

kde4_add_executable(telepathy-kde-text-ui ${telepathy_kde_text_ui_SRCS})

target_link_libraries(telepathy-kde-text-ui
            ${KDE4_KDECORE_LIBS}
            ${KDE4_KDEUI_LIBS}
            ${TELEPATHY_QT4_LIBRARIES}
            ${QT_QTWEBKIT_LIBRARY}
            ${KDE4_KNOTIFYCONFIG_LIBS}
	    ${KDE4_KCMUTILS_LIBS}
            ktelepathy_chat_lib)

configure_file(org.freedesktop.Telepathy.Client.KDE.TextUi.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KDE.TextUi.service)

install(TARGETS telepathy-kde-text-ui ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KDE.TextUi.service
        DESTINATION ${DBUS_SERVICES_INSTALL_DIR})
install(FILES KDE.TextUi.client DESTINATION ${SHARE_INSTALL_PREFIX}/telepathy/clients/)
install(FILES chatwindow.rc
        DESTINATION  ${DATA_INSTALL_DIR}/telepathy-kde-text-ui/)
install(FILES telepathy-kde-text-ui.notifyrc DESTINATION ${DATA_INSTALL_DIR}/telepathy-kde-text-ui)
