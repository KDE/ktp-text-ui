# - Try to find GIO
# Once done this will define
#  GIO_FOUND - System has GIO
#  GIO_INCLUDE_DIRS - The GIO include directories
#  GIO_LIBRARIES - The libraries needed to use GIO
#  GIO_DEFINITIONS - Compiler switches required for using GIO

set(GIO_DEFINITIONS "")

find_path(GIO_INCLUDE_DIR glib-2.0/gio/gio.h)

find_library(GIO_LIBRARY NAMES gio-2.0)

set(GIO_LIBRARIES ${GIO_LIBRARY} )
set(GIO_INCLUDE_DIRS ${GIO_INCLUDE_DIR} ${GIO_INCLUDE_DIR}/glib-2.0/ )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(GIO  DEFAULT_MSG
                                  GIO_LIBRARY GIO_INCLUDE_DIR)

mark_as_advanced(GIO_INCLUDE_DIR GIO_LIBRARY )
