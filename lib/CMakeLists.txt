include_directories (${KTP_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/image-sharer)

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
        contact-delegate.cpp
        notify-filter.cpp
        text-chat-config.cpp
        otr-types.cpp
        channel-adapter.cpp
        otr-channel-proxy.cpp
        proxy-service-interface.cpp
        proxy-service.cpp
        otr-utils.cpp
        authenticationwizard.cpp
        otr-notifications.cpp
        )

set(ktpchat_UI
    chat-widget.ui
    keygendialog.ui
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
    ktpimagesharer
)
install(TARGETS ktpchat ${INSTALL_TARGETS_DEFAULT_ARGS})


install(FILES
        ktptxtui_message_filter.desktop
        DESTINATION
        ${SERVICETYPES_INSTALL_DIR}
)
