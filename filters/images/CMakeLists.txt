add_definitions(-DTRANSLATION_DOMAIN=\"ktp-filters\")

add_library(ktptextui_message_filter_images MODULE images-filter.cpp)

target_link_libraries (ktptextui_message_filter_images
    KF5::Emoticons
    KF5::I18n
    KF5::CoreAddons
    KTp::CommonInternals
)

# Install:
install (TARGETS ktptextui_message_filter_images
         DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_images.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_images.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_images.desktop
         DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
