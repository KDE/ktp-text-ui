# Try to find the KTelepathy library
# KTELEPATHY_FOUND
# KTELEPATHY_INCLUDE_DIR
# KTELEPATHY_LIBRARIES
# KTELEPATHY_MODELS_LIBRARIES

# Copyright (c) 2011, Dario Freddi <drf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (NOT IS_TELEPATHY_KDE_INTERNAL_MODULE)
   message (FATAL_ERROR "KTelepathy can be used just from internal components at this time")
endif (NOT IS_TELEPATHY_KDE_INTERNAL_MODULE)

SET (KTELEPATHY_FIND_REQUIRED ${KTelepathy_FIND_REQUIRED})
if (KTELEPATHY_INCLUDE_DIRS AND KTELEPATHY_LIBRARIES)
  # Already in cache, be silent
  set(KTELEPATHY_FIND_QUIETLY TRUE)
endif (KTELEPATHY_INCLUDE_DIRS AND KTELEPATHY_LIBRARIES)

find_path(KTELEPATHY_INCLUDE_DIR
  NAMES KTelepathy/presence.h
  PATHS ${KDE4_INCLUDE_DIR}
)

find_library(KTELEPATHY_LIBRARIES NAMES telepathykdecommoninternalsprivate )
find_library(KTELEPATHY_MODELS_LIBRARIES NAMES telepathykdemodelsprivate )

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(KTelepathy DEFAULT_MSG
                                  KTELEPATHY_LIBRARIES
                                  KTELEPATHY_MODELS_LIBRARIES
                                  KTELEPATHY_INCLUDE_DIR)

mark_as_advanced(KTELEPATHY_INCLUDE_DIRS KTELEPATHY_LIBRARIES)
