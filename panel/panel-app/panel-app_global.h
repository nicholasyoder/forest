#ifndef PANELAPP_GLOBAL_H
#define PANELAPP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PANELAPP_LIBRARY)
#  define PANELAPP_EXPORT Q_DECL_EXPORT
#else
#  define PANELAPP_EXPORT Q_DECL_IMPORT
#endif

#endif // PANELAPP_GLOBAL_H
