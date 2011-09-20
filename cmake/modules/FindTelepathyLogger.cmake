# - Try to find Telepathy-Logger
# Once done this will define
#
#  TELEPATHY_LOGGER_FOUND - system has Telepathy-Logger
#  TELEPATHY_LOGGER_INCLUDE_DIR - the Telepathy-Logger include directory
#  TELEPATHY_LOGGER_LIBRARIES - the libraries needed to use Telepathy-Logger
#  TELEPATHY_LOGGER_DEFINITIONS - Compiler switches required for using Telepathy-Logger

# Copyright (c) 2011 Collabora Ltd <http://www.collabora.co.uk>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if (TELEPATHY_LOGGER_INCLUDE_DIR AND TELEPATHY_LOGGER_LIBRARIES)
   # in cache already
   set(TELEPATHYLOGGER_FIND_QUIETLY TRUE)
else (TELEPATHY_LOGGER_INCLUDE_DIR AND TELEPATHY_LOGGER_LIBRARIES)
   set(TELEPATHYLOGGER_FIND_QUIETLY FALSE)
endif (TELEPATHY_LOGGER_INCLUDE_DIR AND TELEPATHY_LOGGER_LIBRARIES)

if (NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the find_path() and find_library() calls
    find_package(PkgConfig)
    if (TELEPATHY_LOGGER_MIN_VERSION)
        PKG_CHECK_MODULES(PC_TELEPATHY_LOGGER telepathy-logger-0.2>=${TELEPATHY_LOGGER_MIN_VERSION})
    else (TELEPATHY_LOGGER_MIN_VERSION)
        PKG_CHECK_MODULES(PC_TELEPATHY_LOGGER telepathy-logger-0.2)
    endif (TELEPATHY_LOGGER_MIN_VERSION)
    set(TELEPATHY_LOGGER_DEFINITIONS ${PC_TELEPATHY_LOGGER_CFLAGS_OTHER})
endif (NOT WIN32)

if (TELEPATHY_LOGGER_MIN_VERSION AND PKG_CONFIG_FOUND AND NOT PC_TELEPATHY_LOGGER_FOUND)
    message(STATUS "Telepathy-logger not found or its version is < ${TELEPATHY_LOGGER_MIN_VERSION}")
else (TELEPATHY_LOGGER_MIN_VERSION AND PKG_CONFIG_FOUND AND NOT PC_TELEPATHY_LOGGER_FOUND)
    find_path(TELEPATHY_LOGGER_INCLUDE_DIR telepathy-logger/log-manager.h
       PATHS
       ${PC_TELEPATHY_LOGGER_INCLUDEDIR}
       ${PC_TELEPATHY_LOGGER_INCLUDE_DIRS}
    )

    find_library(TELEPATHY_LOGGER_LIBRARIES NAMES telepathy-logger
       PATHS
       ${PC_TELEPATHY_LOGGER_LIBDIR}
       ${PC_TELEPATHY_LOGGER_LIBRARY_DIRS}
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(TelepathyLogger DEFAULT_MSG TELEPATHY_LOGGER_LIBRARIES
                                                                TELEPATHY_LOGGER_INCLUDE_DIR)

    mark_as_advanced(TELEPATHY_LOGGER_INCLUDE_DIR TELEPATHY_LOGGER_LIBRARIES)

endif (TELEPATHY_LOGGER_MIN_VERSION AND PKG_CONFIG_FOUND AND NOT PC_TELEPATHY_LOGGER_FOUND)
