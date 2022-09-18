add_definitions(-DTRANSLATION_DOMAIN=\"kcm_ktp_chat_behavior\")

set(kcm_ktp_chat_behavior_SRCS behavior-config.cpp)
set(kcm_ktp_chat_behavior_UI behavior-config.ui)

ki18n_wrap_ui(kcm_ktp_chat_behavior_SRCS ${kcm_ktp_chat_behavior_UI})
add_library(kcm_ktp_chat_behavior MODULE ${kcm_ktp_chat_behavior_SRCS})

target_link_libraries(kcm_ktp_chat_behavior
    KF5::TextWidgets
    KF5::KCMUtils
    ktpchat
    ktpimagesharer
)

install(TARGETS
        kcm_ktp_chat_behavior
    DESTINATION ${KDE_INSTALL_PLUGINDIR}
)
install(FILES
        kcm_ktp_chat_behavior.desktop
    DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
