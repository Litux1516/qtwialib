#ifndef QTWIALIB_GLOBAL_H
#define QTWIALIB_GLOBAL_H

#include <Qt/qglobal.h>

#ifdef QTWIALIB_LIB
# define QTWIALIB_EXPORT Q_DECL_EXPORT
#else
# define QTWIALIB_EXPORT Q_DECL_IMPORT
#endif

#endif // QTWIALIB_GLOBAL_H
