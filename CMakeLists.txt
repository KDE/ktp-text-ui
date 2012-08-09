project(ktp-text-ui)

set(IS_KTP_INTERNAL_MODULE TRUE)

# Include our extra FindFoo.cmake files.
set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)

set(KDE_MIN_VERSION "4.4.75")
find_package (KDE4 ${KDE_MIN_VERSION} REQUIRED)
find_package (TelepathyQt4 0.9.1 REQUIRED)
find_package (KTp REQUIRED)

include (CheckIncludeFiles)
include (KDE4Defaults)
include (MacroLibrary)

include_directories (${KDE4_INCLUDES}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
                     ${QT_QTWEBKIT_INCLUDES}
                     ${KTP_INCLUDE_DIR}
                     "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)

add_definitions (${KDE4_DEFINITIONS}
                 -DQT_NO_CAST_FROM_ASCII
                 -DQT_NO_KEYWORDS)

macro_optional_find_package(TelepathyLoggerQt4)
macro_log_feature(TELEPATHY_LOGGER_QT4_FOUND "TelepathyLoggerQt4" "Qt4 wrapper around telepathy-logger" FALSE "" "Needed for optional backlog support in the chat-window")
macro_display_feature_log()
if(TELEPATHY_LOGGER_QT4_FOUND)
    add_definitions( -DTELEPATHY_LOGGER_QT4_FOUND )
    include_directories(
        ${TELEPATHY_LOGGER_QT4_INCLUDE_DIRS}
    )
endif()

add_subdirectory(lib)
add_subdirectory(app)
add_subdirectory(config)
add_subdirectory(data)
add_subdirectory(adiumxtra-protocol-handler)
add_subdirectory(logviewer)
add_subdirectory(plasmoid)


add_subdirectory(filters)
add_subdirectory(tests)

