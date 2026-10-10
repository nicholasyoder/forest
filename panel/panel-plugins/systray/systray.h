// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QHash>
#include <QHBoxLayout>
#include <QWidget>

#include "panelpluginterface.h"

class trayicon;

// StatusNotifierHost: renders the items registered with services-app's watcher.
class systray : public QWidget, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.systray.plugin" FILE "systray.json")
    Q_INTERFACES(panelpluginterface)

public:
    systray();
    ~systray();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<QAction*> itemlist);
    void closePlug(){close(); deleteLater();}
    //end plugininterface

private:
    void registerHost();

private slots:
    // identifier is the StatusNotifierWatcher's "busName+path" string (see
    // services/services-app/systemtray/statusnotifierwatcher.h).
    void addItem(const QString &identifier);
    void removeItem(const QString &identifier);

private:
    QHBoxLayout *mainLayout = nullptr;
    QHash<QString, trayicon*> tIcons;
    // registerHost() runs at setup and again when the watcher appears.
    bool hostRegistered = false;
};

#endif // SYSTRAY_H
