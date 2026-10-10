// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PANELPLUGINTERFACE_H
#define PANELPLUGINTERFACE_H

#include <QObject>
#include <QHBoxLayout>
#include <QVariant>

class QAction;

// Applet info lives in Q_PLUGIN_METADATA's FILE, readable without loading:
// {"name": "Clock", "settings": "desktop/panel/clock", "stretch": true}
class panelpluginterface
{

public:

    //destructor
    virtual ~panelpluginterface() {}

    //called soon after plugin constuctor runs
    virtual void setupPlug(QBoxLayout *, QList<QAction*>)= 0;

    //used when editing what plugins are on the panel
    virtual void closePlug() = 0;

    // Called after the applet's settings page saves.
    virtual void reloadSettings() {}
};

QT_BEGIN_NAMESPACE

Q_DECLARE_INTERFACE(panelpluginterface, "forest.panel.plugin.interface/3")

QT_END_NAMESPACE

#endif // PANELPLUGINTERFACE_H
