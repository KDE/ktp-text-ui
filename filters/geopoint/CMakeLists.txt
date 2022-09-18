add_library(ktptextui_message_filter_geopoint MODULE geopoint-filter.cpp)

target_link_libraries(ktptextui_message_filter_geopoint
    ktpchat
    KF5::KIOWidgets
    KTp::CommonInternals
    )

# Install:
install(TARGETS ktptextui_message_filter_geopoint
    DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_geopoint.desktop.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_geopoint.desktop
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_geopoint.desktop
    DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
