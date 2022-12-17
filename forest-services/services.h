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

#ifndef SERVICES_H
#define SERVICES_H

#include <QObject>
#include <QtDBus>
#include <QLayout>

#include "../library/fpluginterface/fpluginterface.h"
#include "hotkeys/foresthotkeys.h"
#include "notifications/notify.h"
#include "polkit/polkitagent.h"

class services : public QObject, fpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.services.plugin")
    Q_INTERFACES(fpluginterface)

public:
    services();

    //begin pluginterface
    void setupPlug();
    void XcbEventFilter(xcb_generic_event_t *event);
    QHash<QString, QString> getpluginfo();
    //end pluginterface


signals:

public slots:

private:
    foresthotkeys *fhotkeys;
    notify *fnotify;
    polkitagent *fpolkit;
};

#endif // SERVICES_H
