find_package(QJSON REQUIRED)

set (ktptextui_message_filter_urlexpansion_SRCS
     urlexpansion-filter.cpp
)

kde4_add_plugin (ktptextui_message_filter_urlexpansion
                 ${ktptextui_message_filter_urlexpansion_SRCS}
)

target_link_libraries (ktptextui_message_filter_urlexpansion
    ${QT_LIBRARIES}
    ${KDE4_KIO_LIBS}
    ${KTP_LIBRARIES}
    ${TELEPATHY_QT4_LIBRARIES}
    ${QJSON_LIBRARIES}
)

# Install:
install (TARGETS ktptextui_message_filter_urlexpansion
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktptextui_message_filter_urlexpansion.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_urlexpansion.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktptextui_message_filter_urlexpansion.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

install (FILES longurl.js longurlServices.json
         DESTINATION ${DATA_INSTALL_DIR}/ktelepathy)
