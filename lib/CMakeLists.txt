set(telepathy_chat_handler_lib_SRCS
        chatwidget.cpp
        chatwindowstyle.cpp
        chatwindowstylemanager.cpp
        adiumthemeview.cpp
        adiumthemeheaderinfo.cpp
        adiumthememessageinfo.cpp
        adiumthemecontentinfo.cpp
        adiumthemestatusinfo.cpp
        channelcontactlist.cpp
        chatstyleplistfilereader.cpp
        chattextedit.cpp
)

set(telepathy_chat_handler_lib_UI
    chatwidget.ui)

kde4_add_ui_files(telepathy_chat_handler_lib_SRCS ${telepathy_chat_handler_lib_UI})

kde4_add_library(ktelepathy_chat_lib ${telepathy_chat_handler_lib_SRCS})

target_link_libraries(ktelepathy_chat_lib ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${TELEPATHY_QT4_LIBRARIES} ${QT_QTWEBKIT_LIBRARY} ${KDE4_KUTILS_LIBS} )
install(TARGETS ktelepathy_chat_lib ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES ktelepathy.notifyrc DESTINATION ${DATA_INSTALL_DIR}/ktelepathy)
