include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/image-sharer)

set(kcm_ktp_chat_appearance_SRCS appearance-config-tab.cpp appearance-config.cpp)
set(kcm_ktp_chat_behavior_SRCS behavior-config.cpp)
set(kcm_ktp_chat_messages_SRCS messages-config.cpp)
set(kcm_ktp_chat_otr_SRCS otr-config.cpp)

set(kcm_ktp_chat_appearance_UI appearance-config.ui)
set(kcm_ktp_chat_behavior_UI behavior-config.ui)
set(kcm_ktp_chat_otr_UI otr-config.ui)

qt5_wrap_ui(kcm_ktp_chat_appearance_SRCS ${kcm_ktp_chat_appearance_UI})
qt5_wrap_ui(kcm_ktp_chat_behavior_SRCS ${kcm_ktp_chat_behavior_UI})
qt5_wrap_ui(kcm_ktp_chat_otr_SRCS ${kcm_ktp_chat_otr_UI})

add_library(kcm_ktp_chat_appearance MODULE ${kcm_ktp_chat_appearance_SRCS})
add_library(kcm_ktp_chat_behavior MODULE ${kcm_ktp_chat_behavior_SRCS})
add_library(kcm_ktp_chat_messages MODULE ${kcm_ktp_chat_messages_SRCS})
add_library(kcm_ktp_chat_otr MODULE ${kcm_ktp_chat_otr_SRCS})

target_link_libraries(kcm_ktp_chat_appearance
    Qt5::WebKitWidgets
    KF5::KCMUtils
    KF5::WidgetsAddons
    KF5::IconThemes
    ktpchat
)
target_link_libraries(kcm_ktp_chat_behavior
    KF5::TextWidgets
    KF5::KCMUtils
    ktpchat
    ktpimagesharer
)
target_link_libraries(kcm_ktp_chat_messages
    KF5::KCMUtils
    KTp::CommonInternals
    ktpchat
)

target_link_libraries(kcm_ktp_chat_otr
    KF5::KCMUtils
    KF5::WidgetsAddons
    KTp::CommonInternals
    KTp::OTR
    ktpchat
)

install(TARGETS
        kcm_ktp_chat_appearance
        kcm_ktp_chat_behavior
        kcm_ktp_chat_messages
        kcm_ktp_chat_otr
    DESTINATION ${PLUGIN_INSTALL_DIR}
)
install(FILES
        kcm_ktp_chat_appearance.desktop
        kcm_ktp_chat_behavior.desktop
        kcm_ktp_chat_messages.desktop
        kcm_ktp_chat_otr.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
)
