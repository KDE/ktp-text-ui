# - Try to find QtGLib
# Once done this will define
#
#  QTGLIB_FOUND - system has QtGLib
#  QTGLIB_INCLUDE_DIR - the QtGLib include directory
#  QTGLIB_LIBRARIES - the libraries needed to use QtGLib
#  QTGLIB_DEFINITIONS - Compiler switches required for using QtGLib

# Copyright (c) 2011 Collabora Ltd <http://www.collabora.co.uk>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if (QTGLIB_INCLUDE_DIR AND QTGLIB_LIBRARIES)
   # in cache already
   set(QtGLib_FIND_QUIETLY TRUE)
else (QTGLIB_INCLUDE_DIR AND QTGLIB_LIBRARIES)
   set(QtGLib_FIND_QUIETLY FALSE)
endif (QTGLIB_INCLUDE_DIR AND QTGLIB_LIBRARIES)

if (NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the find_path() and find_library() calls
    find_package(PkgConfig)
    if (QTGLIB_MIN_VERSION)
        PKG_CHECK_MODULES(PC_QTGLIB QtGLib-2.0>=${QTGLIB_MIN_VERSION})
    else (QTGLIB_MIN_VERSION)
        PKG_CHECK_MODULES(PC_QTGLIB QtGLib-2.0)
    endif (QTGLIB_MIN_VERSION)
    set(QTGLIB_DEFINITIONS ${PC_QTGLIB_CFLAGS_OTHER})
endif (NOT WIN32)

find_path(QTGLIB_INCLUDE_DIR 
   NAMES QGlib/Object
   PATHS ${PC_QTGLIB_INCLUDEDIR} ${PC_QTGLIB_INCLUDE_DIRS} 
   PATH_SUFFIXES QtGStreamer
   )

find_library(QTGLIB_LIBRARIES 
   NAMES QtGLib-2.0
   PATHS ${PC_QTGLIB_LIBDIR} ${PC_QTGLIB_LIBRARY_DIRS}
   )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QtGLib-2.0 DEFAULT_MSG QTGLIB_LIBRARIES QTGLIB_INCLUDE_DIR)

mark_as_advanced(QTGLIB_INCLUDE_DIR QTGLIB_LIBRARIES)
