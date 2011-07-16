include_directories(${TELEPATHY_KDE_TEXT_UI_SOURCE_DIR}/lib)

set(telepathy_chat_config_SRCS
        main-window.cpp
)

set(telepathy_chat_config_UI
    chatwindowconfig.ui
)

kde4_add_ui_files(telepathy_chat_config_SRCS ${telepathy_chat_config_UI})

kde4_add_plugin(kcm_telepathy_chat_config
		    ${telepathy_chat_config_SRCS})

target_link_libraries(kcm_telepathy_chat_config ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTWEBKIT_LIBRARY} ${KDE4_KCMUTILS_LIBS} ktelepathy_chat_lib)

install(TARGETS kcm_telepathy_chat_config
	  DESTINATION ${PLUGIN_INSTALL_DIR})

install (FILES kcm_telepathy_chat_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)
