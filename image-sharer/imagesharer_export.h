#ifndef IMAGESHARER_EXPORT_H
#define IMAGESHARER_EXPORT_H

// needed for KDE_EXPORT and KDE_IMPORT macros
#include <kdemacros.h>

#ifndef IMAGESHARER_EXPORT
# if defined(MAKE_IMAGESHARER_LIB)
// We are building this library
#  define IMAGESHARER_EXPORT KDE_EXPORT
# else
// We are using this library
#  define IMAGESHARER_EXPORT KDE_IMPORT
# endif
#endif

# ifndef IMAGESHARER_EXPORT_DEPRECATED
#  define IMAGESHARER_EXPORT_DEPRECATED KDE_DEPRECATED IMAGESHARER_EXPORT
# endif

#endif
