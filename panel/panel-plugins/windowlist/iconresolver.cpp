// SPDX-License-Identifier: LGPL-3.0-or-later

#include "iconresolver.h"

#include <QStandardPaths>
#include <qt6xdg/XdgDesktopFile>

QIcon iconresolver::iconForAppId(const QString &appId){
    QIcon fallback = QIcon::fromTheme(appId, QIcon::fromTheme("application-x-executable"));

    if (appId.isEmpty())
        return fallback;

    QString desktopFilePath = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, appId + ".desktop");
    if (desktopFilePath.isEmpty())
        return fallback;

    XdgDesktopFile deskfile;
    deskfile.load(desktopFilePath);
    return deskfile.icon(fallback);
}
