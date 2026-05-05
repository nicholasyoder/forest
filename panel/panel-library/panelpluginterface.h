// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PANELPLUGINTERFACE_H
#define PANELPLUGINTERFACE_H

#include <QObject>
#include <QHBoxLayout>
#include <QVariant>

#include "popupmenu.h"

#include <xcb/xcb.h>

class panelpluginterface
{

public:

    //destructor
    virtual ~panelpluginterface() {}

    //called soon after plugin constuctor runs
    virtual void setupPlug(QBoxLayout *, QList<pmenuitem*>)= 0;

    //used when editing what plugins are on the panel
    virtual void closePlug() = 0;

    //pass xcb events on to plugins
    virtual void XcbEventFilter(xcb_generic_event_t* /*event*/)= 0;

    //should return at least info[name] = plugname
    virtual QHash<QString, QString> getpluginfo() = 0;
};

QT_BEGIN_NAMESPACE

Q_DECLARE_INTERFACE(panelpluginterface, "forest.panel.plugin.interface")

QT_END_NAMESPACE

#endif // PANELPLUGINTERFACE_H
