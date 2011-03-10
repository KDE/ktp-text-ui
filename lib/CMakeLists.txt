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

set(telepathy_chat_handler_lib_HDRS
    adium-theme-content-info.h
    adium-theme-header-info.h
    adium-theme-message-info.h
    adium-theme-status-info.h
    adium-theme-view.h
    chat-style-plist-file-reader.h
    chat-widget.h
    chat-window-style.h
    chat-window-style-manager.h
)

set(telepathy_chat_handler_lib_PRETTY_HDRS
    KDETelepathy/AdiumThemeContentInfo
    KDETelepathy/AdiumThemeHeaderInfo
    KDETelepathy/AdiumThemeMessageInfo
    KDETelepathy/AdiumThemeStatusInfo
    KDETelepathy/AdiumThemeView
    KDETelepathy/ChatStylePlistFileReader
    KDETelepathy/ChatWidget
    KDETelepathy/ChatWindowStyle
    KDETelepathy/ChatWindowStyleManager
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

install(FILES
        ${telepathy_chat_handler_lib_HDRS}
        ${telepathy_chat_handler_lib_PRETTY_HDRS}
        kdetelepathychat_export.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/KDETelepathy COMPONENT Devel
)


install( FILES ktelepathy.notifyrc DESTINATION ${DATA_INSTALL_DIR}/ktelepathy)
