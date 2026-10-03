// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PANELPLUGINTERFACE_H
#define PANELPLUGINTERFACE_H

#include <QObject>
#include <QHBoxLayout>
#include <QVariant>

class QAction;

class panelpluginterface
{

public:

    //destructor
    virtual ~panelpluginterface() {}

    //called soon after plugin constuctor runs
    virtual void setupPlug(QBoxLayout *, QList<QAction*>)= 0;

    //used when editing what plugins are on the panel
    virtual void closePlug() = 0;

    //should return at least info[name] = plugname
    virtual QHash<QString, QString> getpluginfo() = 0;
};

QT_BEGIN_NAMESPACE

Q_DECLARE_INTERFACE(panelpluginterface, "forest.panel.plugin.interface/2")

QT_END_NAMESPACE

#endif // PANELPLUGINTERFACE_H
