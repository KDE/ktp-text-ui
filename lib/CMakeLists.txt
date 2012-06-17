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
        logmanager.cpp
        message.cpp
        message-processor.cpp
        url-filter.cpp
        emoticon-filter.cpp
        escape-filter.cpp
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
)

set(ktpchat_UI
    chat-widget.ui
)

kde4_add_ui_files(ktpchat_SRCS ${ktpchat_UI})

kde4_add_library(ktpchat SHARED ${ktpchat_SRCS})

# telepathy-logger-qt4 related stuff
if(TELEPATHY_LOGGER_QT4_FOUND)
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TELEPATHY_LOGGER_QT4_DEFINITIONS}" )

    include_directories(
        ${TELEPATHY_LOGGER_QT4_INCLUDE_DIRS}
    )

    target_link_libraries(ktpchat
        ${TELEPATHY_LOGGER_QT4_LIBRARIES}
    )
endif()

target_link_libraries(ktpchat
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${QT_QTWEBKIT_LIBRARY}
    ${KDE4_KEMOTICONS_LIBS}
    ${KTP_LIBRARIES}
)
install(TARGETS ktpchat ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
        ${ktpchat_HDRS}
        ${ktpchat_PRETTY_HDRS}
        ktpchat_export.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/KTp COMPONENT Devel
)

