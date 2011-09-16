include_directories(${TELEPATHY_KDE_TEXT_UI_SOURCE_DIR}/lib)

set(telepathy_chat_config_SRCS
        bundle-installer.cpp
        chat-style-installer.cpp
        emoticon-set-installer.cpp
        adiumxtra-protocol-handler.cpp
        main.cpp
)



kde4_add_ui_files(telepathy_chat_config_SRCS)

kde4_add_executable(adiumxtra-protocol-handler ${telepathy_chat_config_SRCS})

target_link_libraries(adiumxtra-protocol-handler
			${KDE4_KDECORE_LIBS}
			${KDE4_KDEUI_LIBS}
			${QT_QTWEBKIT_LIBRARY}
			${KDE4_KEMOTICONS_LIBS}
			${QT_QTXML_LIBRARY}
			${KDE4_KIO_LIBS}
			ktelepathy_chat_lib)


install(TARGETS adiumxtra-protocol-handler DESTINATION ${LIBEXEC_INSTALL_DIR})

configure_file(adiumxtra.protocol.in  ${CMAKE_CURRENT_BINARY_DIR}/adiumxtra.protocol)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/adiumxtra.protocol DESTINATION ${SERVICES_INSTALL_DIR})
