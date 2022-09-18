add_library (ktptextui_message_filter_urlexpansion MODULE urlexpansion-filter.cpp)

target_link_libraries (ktptextui_message_filter_urlexpansion
    KF5::KIOWidgets
    KF5::CoreAddons
    KTp::CommonInternals
)

# Install:
install (TARGETS ktptextui_message_filter_urlexpansion
         DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_urlexpansion.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_urlexpansion.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_urlexpansion.desktop
         DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)

install (FILES longurl.js longurlServices.json
         DESTINATION ${KDE_INSTALL_DATADIR}/ktelepathy)
