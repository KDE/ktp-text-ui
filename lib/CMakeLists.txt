include_directories(${CMAKE_SOURCE_DIR}/image-sharer)

add_definitions(-DTRANSLATION_DOMAIN=\"ktpchat\")

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
        ktp-debug.cpp
        )

set(ktpchat_UI
    chat-widget.ui
    keygendialog.ui
)

ki18n_wrap_ui(ktpchat_SRCS ${ktpchat_UI})

add_library(ktpchat SHARED ${ktpchat_SRCS})

target_link_libraries(ktpchat
    KF5::KIOWidgets
    KF5::KIOCore
    KF5::I18n
    KF5::WebKit
    KF5::Emoticons
    KF5::IconThemes
    KF5::Archive
    KF5::SonnetCore
    KF5::Notifications
    KF5::TextWidgets
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::WindowSystem
    KF5::XmlGui #for KActionCollection
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
