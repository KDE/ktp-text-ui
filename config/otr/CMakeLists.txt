add_definitions(-DTRANSLATION_DOMAIN=\"kcm_ktp_chat_otr\")

set(kcm_ktp_chat_otr_SRCS otr-config.cpp)
set(kcm_ktp_chat_otr_UI otr-config.ui)

ki18n_wrap_ui(kcm_ktp_chat_otr_SRCS ${kcm_ktp_chat_otr_UI})
add_library(kcm_ktp_chat_otr MODULE ${kcm_ktp_chat_otr_SRCS})

target_link_libraries(kcm_ktp_chat_otr
    KF5::KCMUtils
    KF5::WidgetsAddons
    KTp::CommonInternals
    KTp::OTR
    ktpchat
)

install(TARGETS
        kcm_ktp_chat_otr
    DESTINATION ${KDE_INSTALL_PLUGINDIR}
)
install(FILES
        kcm_ktp_chat_otr.desktop
    DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
