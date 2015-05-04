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
    DESTINATION ${PLUGIN_INSTALL_DIR}
)
install(FILES
        kcm_ktp_chat_messages.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
)
