add_library(ktptextui_message_filter_youtube MODULE youtube-filter.cpp)

target_link_libraries (ktptextui_message_filter_youtube
    ktpchat
    KF5::KIOWidgets
    KTp::CommonInternals
)

# Install:
install (TARGETS ktptextui_message_filter_youtube
         DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_youtube.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_youtube.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_youtube.desktop
         DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
