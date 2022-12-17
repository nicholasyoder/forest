/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QSettings>
#include <QProcess>
#include <QDebug>

#include "animwidget.h"

class session : public QObject
{
    Q_OBJECT
public:
    session();

signals:

public slots:
    void startsession();

    void loadEnvironmentSettings();
    void loadKeyboardSettings();
    void loadMouseSettings();

    void runautostart();

private:
    QSettings *settings;
};

#endif // SESSION_H
