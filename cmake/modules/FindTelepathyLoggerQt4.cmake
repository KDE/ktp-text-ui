# - Try to find TelepathyLoggerQt4
# Once done this will define
#  TELEPATHY_LOGGER_QT4_FOUND - System has TelepathyLoggerQt4
#  TELEPATHY_LOGGER_QT4_INCLUDE_DIRS - The TelepathyLoggerQt4 include directories
#  TELEPATHY_LOGGER_QT4_LIBRARIES - The libraries needed to use TelepathyLoggerQt4
#  TELEPATHY_LOGGER_QT4_DEFINITIONS - Compiler switches required for using TelepathyLoggerQt4

# Copyright (c) 2010 Dominik Schmidt <kde@dominik-schmidt.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(TELEPATHY_LOGGER_QT4_DEFINITIONS "-fexceptions")

find_path(TELEPATHY_LOGGER_QT4_INCLUDE_DIR telepathy-logger-0.2/TelepathyLoggerQt4/init.h)

find_library(TELEPATHY_LOGGER_QT4_LIBRARY NAMES libtelepathy-logger-qt4 telepathy-logger-qt4)

set(TELEPATHY_LOGGER_QT4_LIBRARIES ${TELEPATHY_LOGGER_QT4_LIBRARY} )
set(TELEPATHY_LOGGER_QT4_INCLUDE_DIRS ${TELEPATHY_LOGGER_QT4_INCLUDE_DIR} ${TELEPATHY_LOGGER_QT4_INCLUDE_DIR}/telepathy-logger-0.2 )

find_package(TelepathyLogger)
find_package(TelepathyGlib)
find_package(GObject)
find_package(GIO)
find_package(GLIB2)
find_package(QtGLib)

list(APPEND TELEPATHY_LOGGER_QT4_LIBRARIES
    ${TELEPATHY_LOGGER_LIBRARIES}
    ${TELEPATHY_GLIB_LIBRARIES}
    ${GLIB2_LIBRARIES}
    ${GOBJECT_LIBRARIES}
    ${QTGLIB_LIBRARIES}
    ${GIO_LIBRARIES}
)

list(APPEND TELEPATHY_LOGGER_QT4_INCLUDE_DIRS
    ${GLIB2_INCLUDE_DIR}
    ${QTGLIB_INCLUDE_DIR}
)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(TELEPATHY_LOGGER_QT4  DEFAULT_MSG
                                  TELEPATHY_LOGGER_QT4_LIBRARY TELEPATHY_LOGGER_QT4_INCLUDE_DIR)

mark_as_advanced(TELEPATHY_LOGGER_QT4_INCLUDE_DIRS TELEPATHY_LOGGER_QT4_LIBRARY )
