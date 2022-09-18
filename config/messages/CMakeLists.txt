add_definitions(-DTRANSLATION_DOMAIN=\"kcm_ktp_chat_messages\")

set(kcm_ktp_chat_messages_SRCS messages-config.cpp)

add_library(kcm_ktp_chat_messages MODULE ${kcm_ktp_chat_messages_SRCS})

target_link_libraries(kcm_ktp_chat_messages
    KF5::KCMUtils
    KTp::CommonInternals
    ktpchat
)

install(TARGETS
        kcm_ktp_chat_messages
    DESTINATION ${KDE_INSTALL_PLUGINDIR}
)
install(FILES
        kcm_ktp_chat_messages.desktop
    DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
