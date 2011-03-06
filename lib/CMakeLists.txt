set(telepathy_chat_handler_lib_SRCS
        chat-widget.cpp
        chat-window-style.cpp
        chat-window-style-manager.cpp
        adium-theme-view.cpp
        adium-theme-header-info.cpp
        adium-theme-message-info.cpp
        adium-theme-content-info.cpp
        adium-theme-status-info.cpp
        channel-contact-list.cpp
        chat-style-plist-file-reader.cpp
        chat-text-edit.cpp
)

set(telepathy_chat_handler_lib_UI
    chat-widget.ui)

kde4_add_ui_files(telepathy_chat_handler_lib_SRCS ${telepathy_chat_handler_lib_UI})

kde4_add_library(ktelepathy_chat_lib SHARED ${telepathy_chat_handler_lib_SRCS})

target_link_libraries(ktelepathy_chat_lib
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${QT_QTWEBKIT_LIBRARY}
    ${KDE4_KEMOTICONS_LIBS}
    ${KDE4_KCMUTILS_LIBS}
)
install(TARGETS ktelepathy_chat_lib ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES ktelepathy.notifyrc DESTINATION ${DATA_INSTALL_DIR}/ktelepathy)
