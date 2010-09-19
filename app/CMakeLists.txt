# FIXME when the chat connection builds a lib correctly, don't do this weird horrible hack.

include_directories(../lib/)

set(telepathy_chat_handler_SRCS
        main.cpp
        mainwindow.cpp
)

kde4_add_executable(telepathy-chat-handler ${telepathy_chat_handler_SRCS})

target_link_libraries(telepathy-chat-handler ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${TELEPATHY_QT4_LIBRARIES} ${QT_QTWEBKIT_LIBRARY} ${KDE4_KUTILS_LIBS} ktelepathy_chat_lib)

configure_file(org.freedesktop.Telepathy.Client.KDEChatHandler.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KDEChatHandler.service)

install(TARGETS telepathy-chat-handler ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KDEChatHandler.service
        DESTINATION ${DBUS_SERVICES_INSTALL_DIR})
install(FILES KDEChatHandler.client DESTINATION ${SHARE_INSTALL_PREFIX}/telepathy/clients/)
