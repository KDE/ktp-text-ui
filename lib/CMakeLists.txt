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
        proxy-service.cpp
        otr-status.cpp
        authenticationwizard.cpp
        otr-notifications.cpp
        )

set(ktpchat_UI
    chat-widget.ui
    keygendialog.ui
)

qt5_wrap_ui(ktpchat_SRCS ${ktpchat_UI})

add_library(ktpchat SHARED ${ktpchat_SRCS})

set_target_properties(ktpchat PROPERTIES SOVERSION ${KTP_TEXT_UI_VERSION} VERSION ${KTP_TEXT_UI_VERSION})

target_link_libraries(ktpchat
    KF5::KIOWidgets
    KF5::WebKit
    KF5::Emoticons
    KF5::IconThemes
    KF5::Archive
    KF5::SonnetCore
    KTp::CommonInternals
    KTp::Logger
    KTp::OTR
    KTp::Widgets
    Qt5::WebKitWidgets
    ktpimagesharer
)
install(TARGETS ktpchat ${INSTALL_TARGETS_DEFAULT_ARGS})


install(FILES
        ktptxtui_message_filter.desktop
        DESTINATION
        ${SERVICETYPES_INSTALL_DIR}
)
