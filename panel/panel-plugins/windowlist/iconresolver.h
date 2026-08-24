// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ICONRESOLVER_H
#define ICONRESOLVER_H

#include <QIcon>
#include <QString>

// Resolves a Wayland toplevel's app_id to an icon. Unlike X11's
// _NET_WM_ICON, wlr-foreign-toplevel-management carries no icon data at
// all - only app_id - so this has to go through the app's .desktop file,
// same pattern already used in quicklaunch/launcher.cpp and
// mainmenu/itemhandler.cpp.
namespace iconresolver{
    QIcon iconForAppId(const QString &appId);
}

#endif // ICONRESOLVER_H
