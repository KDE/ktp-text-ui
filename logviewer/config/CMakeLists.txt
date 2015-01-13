include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/logviewer)

set(kcm_ktp_logviewer_behavior_SRCS behavior-config.cpp)

set(kcm_ktp_logviewer_behavior_UI behavior-config.ui)

qt5_wrap_ui(kcm_ktp_logviewer_behavior_SRCS ${kcm_ktp_logviewer_behavior_UI})

add_library(kcm_ktp_logviewer_behavior MODULE ${kcm_ktp_logviewer_behavior_SRCS})

target_link_libraries(kcm_ktp_logviewer_behavior
    KF5::KCMUtils
    KF5::KDELibs4Support
    ktpchat
)

install(TARGETS
        kcm_ktp_logviewer_behavior
    DESTINATION ${PLUGIN_INSTALL_DIR}
)
install(FILES
        kcm_ktp_logviewer_behavior.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
)
