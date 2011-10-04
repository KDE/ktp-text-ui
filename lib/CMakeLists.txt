macro_optional_find_package(TelepathyLoggerQt4)
macro_log_feature(TELEPATHY_LOGGER_QT4_FOUND "TelepathyLoggerQt4" "Qt4 wrapper around telepathy-logger" FALSE "" "Needed for optional backlog support in the chat-window")
macro_display_feature_log()
if(TELEPATHY_LOGGER_QT4_FOUND)
    add_definitions( -DTELEPATHY_LOGGER_QT4_FOUND )
endif()

set(telepathy_chat_handler_lib_SRCS
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
        logmanager.cpp
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
    chat-widget.ui
)

kde4_add_ui_files(telepathy_chat_handler_lib_SRCS ${telepathy_chat_handler_lib_UI})

kde4_add_library(ktelepathy_chat_lib SHARED ${telepathy_chat_handler_lib_SRCS})

# telepathy-logger-qt4 related stuff
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TELEPATHY_LOGGER_QT4_DEFINITIONS}" )
include_directories(
    ${TELEPATHY_LOGGER_QT4_INCLUDE_DIRS}
)

target_link_libraries(ktelepathy_chat_lib
    ${TELEPATHY_LOGGER_QT4_LIBRARIES}

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
