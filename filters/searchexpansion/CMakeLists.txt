add_library (ktptextui_message_filter_searchexpansion MODULE searchexpansion-filter.cpp)

target_link_libraries (ktptextui_message_filter_searchexpansion
    ktpchat
    KF5::KIOWidgets
    KF5::CoreAddons
    KTp::CommonInternals
)

# Install:
install (TARGETS ktptextui_message_filter_searchexpansion
         DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_searchexpansion.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_searchexpansion.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_searchexpansion.desktop
         DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)

