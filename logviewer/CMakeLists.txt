macro_optional_find_package(TelepathyLoggerQt4)
macro_log_feature(TELEPATHY_LOGGER_QT4_FOUND "TelepathyLoggerQt4" "Qt4 wrapper around telepathy-logger" FALSE "" "Needed for optional backlog support in the chat-window")
macro_display_feature_log()

#Only do anything with this app if the logger is available.
if(TELEPATHY_LOGGER_QT4_FOUND)
    add_definitions( -DTELEPATHY_LOGGER_QT4_FOUND )


    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TELEPATHY_LOGGER_QT4_DEFINITIONS}" )


include_directories(${CMAKE_SOURCE_DIR}/lib
                    ${TELEPATHY_LOGGER_QT4_INCLUDE_DIRS}
                   )

set(ktp-log-viewer_SRCS
        main.cpp
        log-viewer.cpp
        entity-model.cpp
        message-view.cpp
)

kde4_add_ui_files(ktp-log-viewer_SRCS log-viewer.ui)

kde4_add_executable(ktp-log-viewer ${ktp-log-viewer_SRCS})

target_link_libraries(ktp-log-viewer
            ${KDE4_KDECORE_LIBS}
            ${KDE4_KDEUI_LIBS}
            ${KDE4_KIO_LIBS}
            ${TELEPATHY_QT4_LIBRARIES}
            ${KDE4_KEMOTICONS_LIBS}
	    ${KDE4_KCMUTILS_LIBS}
            ${TELEPATHY_LOGGER_QT4_LIBRARIES}
            ktpchat)


# change to binary install dir
# install(TARGETS ktp-log-viewer DESTINATION ${LIBEXEC_INSTALL_DIR})
endif()
