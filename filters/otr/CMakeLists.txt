set (ktptextui_message_filter_otr_SRCS
     otr-filter.cpp
)

kde4_add_plugin (ktptextui_message_filter_otr
                 ${ktptextui_message_filter_otr_SRCS}
)

target_link_libraries (ktptextui_message_filter_otr
    ${QT_LIBRARIES}
    ${KDE4_KIO_LIBS}
    ${KTP_LIBRARIES}
    ${KTP_OTR_LIBRARIES}
)

# Install:
install (TARGETS ktptextui_message_filter_otr
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_otr.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_otr.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_otr.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)
