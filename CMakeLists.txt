project(ktp-text-ui)

set(KTP_TEXT_UI_VERSION "0.7.80")
set(KTP_MESSAGE_FILTER_FRAMEWORK_VERSION "4")
set(IS_KTP_INTERNAL_MODULE TRUE)

# Include our extra FindFoo.cmake files.
set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)
# cmake_policy(SET CMP0002 OLD)

set(KDE_MIN_VERSION "4.4.75")
find_package (KDE4 ${KDE_MIN_VERSION} REQUIRED)
find_package (TelepathyQt4 0.9.1 REQUIRED)
find_package (KTp REQUIRED)
find_package (KPeople 0.2)


macro_log_feature(KPEOPLE_FOUND "KPeople" "Support for KDE Contact Aggregation" "https://projects.kde.org/libkpeople" FALSE "" "")
macro_display_feature_log()

if (KPEOPLE_FOUND)
    add_definitions(-DHAVE_KPEOPLE)
    find_package (KdepimLibs REQUIRED)
    include_directories(${KPEOPLE_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIRS})
endif (KPEOPLE_FOUND)

# FUTURE: When we can depend on KDE 4.10, use emoticoncontexteditaction from here
# find_package(KdepimLibs 4.10 REQUIRED)

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

configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(image-sharer)
add_subdirectory(lib)
add_subdirectory(app)
add_subdirectory(config)
add_subdirectory(data)
add_subdirectory(adiumxtra-protocol-handler)
add_subdirectory(filters)
add_subdirectory(logviewer)
