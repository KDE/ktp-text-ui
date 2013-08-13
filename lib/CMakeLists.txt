include_directories (${KTP_INCLUDE_DIR})

set(ktpchat_SRCS
        chat-widget.cpp
        chat-window-style.cpp
        chat-window-style-manager.cpp
        adium-theme-view.cpp
        adium-theme-header-info.cpp
        adium-theme-message-info.cpp
        adium-theme-content-info.cpp
        adium-theme-status-info.cpp
        channel-contact-model.cpp
        chat-style-plist-file-reader.cpp
        chat-text-edit.cpp
        chat-search-bar.cpp
        notify-filter.cpp
        text-chat-config.cpp
        logmanager.cpp
)

set(ktpchat_HDRS
    adium-theme-content-info.h
    adium-theme-header-info.h
    adium-theme-message-info.h
    adium-theme-status-info.h
    adium-theme-view.h
    chat-style-plist-file-reader.h
    chat-widget.h
    chat-window-style.h
    chat-window-style-manager.h
    text-chat-config.h
)

set(ktpchat_PRETTY_HDRS
    KTp/AdiumThemeContentInfo
    KTp/AdiumThemeHeaderInfo
    KTp/AdiumThemeMessageInfo
    KTp/AdiumThemeStatusInfo
    KTp/AdiumThemeView
    KTp/ChatStylePlistFileReader
    KTp/ChatWidget
    KTp/ChatWindowStyle
    KTp/ChatWindowStyleManager
    KTp/TextChatConfig
)

set(ktpchat_UI
    chat-widget.ui
)

kde4_add_ui_files(ktpchat_SRCS ${ktpchat_UI})

kde4_add_library(ktpchat SHARED ${ktpchat_SRCS})

target_link_libraries(ktpchat
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KDEWEBKIT_LIBRARY}
    ${KDE4_KEMOTICONS_LIBS}
    ${KTP_LIBRARIES}
    ${KTP_LOGGER_LIBRARIES}
)
install(TARGETS ktpchat ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
        ${ktpchat_HDRS}
        ${ktpchat_PRETTY_HDRS}
        ktpchat_export.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/KTp COMPONENT Devel
)

install(FILES
        ktptxtui_message_filter.desktop
        DESTINATION
        ${SERVICETYPES_INSTALL_DIR}
)
