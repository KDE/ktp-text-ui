project (telepathy_chat_handler)

# Include our extra FindFoo.cmake files.
set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)

set(KDE_MIN_VERSION "4.4.75")
find_package (KDE4 ${KDE_MIN_VERSION} REQUIRED)
find_package (Nepomuk REQUIRED)
find_package (TelepathyQt4 REQUIRED)
set(SDO_MIN_VERSION "0.3.60")
find_package(SharedDesktopOntologies ${SDO_MIN_VERSION} REQUIRED)

include (CheckIncludeFiles)
include (KDE4Defaults)
include (MacroLibrary)

include_directories (${KDE4_INCLUDES}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
                     ${NEPOMUK_INCLUDES}
                     ${QT_QTWEBKIT_INCLUDES}
)


set(telepathy_chat_handler_SRCS
        main.cpp
        mainwindow.cpp
        realclienthandler.cpp
        chatconnection.cpp
        chatwindow.cpp
        chatwindowstyle.cpp
        chatview.cpp
        telepathychatmessageinfo.cpp
        telepathychatinfo.cpp
)


set(telepathy_chat_handler_UI
    chatwindow.ui
    mainwindow.ui
)

kde4_add_ui_files(telepathy_chat_handler_SRCS ${telepathy_chat_handler_UI})

kde4_add_executable(telepathy-chat-handler ${telepathy_chat_handler_SRCS})

target_link_libraries(telepathy-chat-handler ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${TELEPATHY_QT4_LIBRARIES} ${QT_QTWEBKIT_LIBRARY} ${KDE4_KUTILS_LIBS} )

configure_file(org.freedesktop.Telepathy.Client.KDEChatHandler.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KDEChatHandler.service)

install(TARGETS telepathy-chat-handler ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KDEChatHandler.service
        DESTINATION ${DBUS_SERVICES_INSTALL_DIR})
install(FILES KDEChatHandler.client DESTINATION ${SHARE_INSTALL_PREFIX}/telepathy/clients/)
