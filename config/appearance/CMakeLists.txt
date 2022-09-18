add_definitions(-DTRANSLATION_DOMAIN=\"kcm_ktp_chat_appearance\")

set(kcm_ktp_chat_appearance_SRCS appearance-config-tab.cpp appearance-config.cpp)
set(kcm_ktp_chat_appearance_UI appearance-config.ui)

ki18n_wrap_ui(kcm_ktp_chat_appearance_SRCS ${kcm_ktp_chat_appearance_UI})
add_library(kcm_ktp_chat_appearance MODULE ${kcm_ktp_chat_appearance_SRCS})

target_link_libraries(kcm_ktp_chat_appearance
    Qt5::WebEngineWidgets
    KF5::KCMUtils
    KF5::WidgetsAddons
    KF5::IconThemes
    ktpchat
)

install(TARGETS
        kcm_ktp_chat_appearance
    DESTINATION ${KDE_INSTALL_PLUGINDIR}
)
install(FILES
        kcm_ktp_chat_appearance.desktop
    DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
