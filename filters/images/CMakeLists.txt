set (ktptextui_message_filter_images_SRCS
     images-filter.cpp
)

kde4_add_ui_files (ktptextui_message_filter_images_SRCS
)

kde4_add_plugin (ktptextui_message_filter_images
                 ${ktptextui_message_filter_images_SRCS}
)

target_link_libraries (ktptextui_message_filter_images
    ktpchat
    ${QT_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KEMOTICONS_LIBS}
)

# Install:
install (TARGETS ktptextui_message_filter_images
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES ktptextui_message_filter_images.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

