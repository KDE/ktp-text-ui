include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/image-sharer)

set(ktp-text-ui_SRCS
        main.cpp
        telepathy-chat-ui.cpp
        chat-window.cpp
        chat-tab.cpp
        emoticon-text-edit-action.cpp
        emoticon-text-edit-selector.cpp
        invite-contact-dialog.cpp
)

set (ktp-text-ui_LIBS
        Qt5::WebEngine
        KF5::CoreAddons
        KF5::I18n
        KF5::KIOWidgets
        KF5::ConfigWidgets
        KF5::Emoticons
        KF5::WidgetsAddons
        KTp::CommonInternals
        KTp::Models
        KTp::Widgets
        KTp::OTR
        KF5::NotifyConfig
        KF5::KCMUtils
        ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
        ktpchat
)

if(KF5People_FOUND)
    set (ktp-text-ui_LIBS
        ${ktp-text-ui_LIBS}
        KF5::PeopleWidgets
    )
endif()

add_executable(ktp-text-ui ${ktp-text-ui_SRCS})

target_link_libraries(ktp-text-ui
            ${ktp-text-ui_LIBS})

configure_file(org.freedesktop.Telepathy.Client.KTp.TextUi.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.TextUi.service)

install(TARGETS ktp-text-ui DESTINATION ${KDE_INSTALL_LIBEXECDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.TextUi.service
        DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})
install(FILES KTp.TextUi.client DESTINATION ${KDE_INSTALL_DATAROOTDIR}/telepathy/clients/)
install(FILES chatwindow.rc
  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/ktp-text-ui/)
