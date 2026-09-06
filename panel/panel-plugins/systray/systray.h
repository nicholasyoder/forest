// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QHash>
#include <QHBoxLayout>
#include <QWidget>

#include "panelpluginterface.h"

class trayicon;

// Host half of the StatusNotifierItem tray: registers with the
// StatusNotifierWatcher (services/services-app/systemtray, always running
// independently of this plugin) and renders whatever items are currently
// registered there as trayicon widgets.
class systray : public QWidget, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.systray.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    systray();
    ~systray();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){close(); deleteLater();}
    void XcbEventFilter(xcb_generic_event_t*){}
    QHash<QString, QString> getpluginfo();
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
    // Guards against registering/connecting twice - registerHost() is
    // called both eagerly from setupPlug() and again if a QDBusServiceWatcher
    // (see systray.cpp) later reports the watcher actually appearing, since
    // services-app (which owns it) has no guaranteed load-order relative to
    // panel-app.
    bool hostRegistered = false;
};

#endif // SYSTRAY_H
