// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ICONRESOLVER_H
#define ICONRESOLVER_H

#include <QIcon>
#include <QString>

// app_id -> icon via the app's .desktop file; foreign-toplevel carries no icon data.
namespace iconresolver{
    QIcon iconForAppId(const QString &appId);
}

#endif // ICONRESOLVER_H
