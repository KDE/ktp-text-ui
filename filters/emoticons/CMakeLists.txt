add_library (ktptextui_message_filter_emoticons MODULE emoticon-filter.cpp)

target_link_libraries (ktptextui_message_filter_emoticons
    ktpchat
    KF5::Emoticons
    KF5::CoreAddons
    KTp::CommonInternals
)

ki18n_wrap_ui(kcm_ktptextui_message_filter_emoticons_SRCS
    emoticon-config.ui
)

add_library(kcm_ktptextui_message_filter_emoticons MODULE emoticon-config.cpp
                 ${kcm_ktptextui_message_filter_emoticons_SRCS}
)

kcoreaddons_desktop_to_json(kcm_ktptextui_message_filter_emoticons kcm_ktptextui_message_filter_emoticons.desktop)

target_link_libraries (kcm_ktptextui_message_filter_emoticons
    ktpchat
    KF5::KCMUtils
    KF5::ItemViews
    KF5::Emoticons
    KTp::CommonInternals
)

# Install:
install (TARGETS ktptextui_message_filter_emoticons
                 kcm_ktptextui_message_filter_emoticons
         DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_emoticons.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_emoticons.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_emoticons.desktop
               ${CMAKE_CURRENT_SOURCE_DIR}/kcm_ktptextui_message_filter_emoticons.desktop
         DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
