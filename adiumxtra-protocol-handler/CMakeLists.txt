# FIXME when the chat connection builds a lib correctly, don't do this weird horrible hack.

include_directories(../lib/)

set(telepathy_chat_config_SRCS
        bundleinstaller.cpp
        chatstyleinstaller.cpp
        emoticonsetinstaller.cpp
        adiumxtraprotocolhandler.cpp
        main.cpp
)



kde4_add_ui_files(telepathy_chat_config_SRCS)

kde4_add_executable(adiumxtra-protocol-handler ${telepathy_chat_config_SRCS})

target_link_libraries(adiumxtra-protocol-handler
			${KDE4_KDECORE_LIBS}
			${KDE4_KDEUI_LIBS}
			${QT_QTWEBKIT_LIBRARY}
			${KDE4_KUTILS_LIBS}
			${QT_QTXML_LIBRARY}
			${KDE4_KIO_LIBRARIES}
			ktelepathy_chat_lib)


install(TARGETS adiumxtra-protocol-handler ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES adiumxtra.protocol DESTINATION ${SERVICES_INSTALL_DIR} )

install(FILES adiumxtra-protocol-handler.notifyrc DESTINATION ${DATA_INSTALL_DIR}/adiumxtra-protocol-handler)
