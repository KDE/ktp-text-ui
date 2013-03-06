set (ktptextui_message_filter_searchexpansion_SRCS
     searchexpansion-filter.cpp
)

kde4_add_plugin (ktptextui_message_filter_searchexpansion
                 ${ktptextui_message_filter_searchexpansion_SRCS}
)

target_link_libraries (ktptextui_message_filter_searchexpansion
    ktpchat
    ${QT_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KEMOTICONS_LIBS}
    ${KTP_LIBRARIES}
)

# Install:
install (TARGETS ktptextui_message_filter_searchexpansion
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_searchexpansion.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_searchexpansion.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_searchexpansion.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

