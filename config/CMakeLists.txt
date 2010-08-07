# FIXME when the chat connection builds a lib correctly, don't do this weird horrible hack.

include_directories(../lib/)

set(telepathy_chat_config_SRCS
        main.cpp
        mainwindow.cpp
        ../lib/chatwindowstylemanager.cpp
        ../lib/chatwindowstyle.cpp
        ../lib/chatview.cpp
        ../lib/telepathychatmessageinfo.cpp
        ../lib/telepathychatinfo.cpp
)

set(telepathy_chat_config_UI
    ../lib/chatwindow.ui
    chatwindowconfig.ui
)

kde4_add_ui_files(telepathy_chat_config_SRCS ${telepathy_chat_config_UI})

kde4_add_executable(telepathy-chat-config ${telepathy_chat_config_SRCS})

target_link_libraries(telepathy-chat-config ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTWEBKIT_LIBRARY} ${KDE4_KUTILS_LIBS} )

