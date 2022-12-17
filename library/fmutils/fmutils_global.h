#ifndef FMUTILS_GLOBAL_H
#define FMUTILS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(FMUTILS_LIBRARY)
#  define FMUTILSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define FMUTILSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // FMUTILS_GLOBAL_H
