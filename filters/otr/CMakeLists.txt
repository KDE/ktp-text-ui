add_library (ktptextui_message_filter_otr MODULE otr-filter.cpp)

target_link_libraries (ktptextui_message_filter_otr
    KF5::KIOWidgets
    KF5::I18n
    KF5::CoreAddons
    KTp::CommonInternals
    KTp::OTR
)

# Install:
install (TARGETS ktptextui_message_filter_otr
         DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_otr.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_otr.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_otr.desktop
         DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
