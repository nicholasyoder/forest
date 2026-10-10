// SPDX-License-Identifier: LGPL-3.0-or-later

#include "panel.h"

#include <QJsonObject>

#include "panelconfig.h"
#include "xdgactivation.h"

panel::panel(){}

panel::~panel(){}

void panel::setupPlug(){
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);

    geometry_manager = new GeometryManager(this);

    wlayout = new QHBoxLayout;
    wlayout->setContentsMargins(QMargins(0,0,0,0));
    wlayout->setSpacing(0);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(QMargins(0,0,0,0));
    pframe = new panelQFrame();
    pframe->setObjectName("panel");
    pframe->setLayout(wlayout);
    vlayout->addWidget(pframe);
    connect(pframe, &panelQFrame::resized, this, &panel::update_panel_size);

    loadsettings();
    loadplugins();

    if (!QDBusConnection::sessionBus().registerObject("/org/forest/panel", this, QDBusConnection::ExportAllSlots))
        qCritical() << "Failed to register /org/forest/panel on DBus:" << QDBusConnection::sessionBus().lastError().message();

    show();
}

void panel::showsettings(){
    XdgActivation::instance()->launch("forest-settings", {"desktop/panel"});
}

void panel::loadsettings(){
    bool autohide = settings->value(panelconfig::autohide, panelconfig::autohide_default).toBool();

    // only reserve screen space for panel if it is always visible
    geometry_manager->set_reserve_screen_space(!autohide);

    if (autohide && !autohide_manager){
        autohide_manager = new AutoHideManager(geometry_manager, this);
        autohide_manager->start();
    }
    else if (!autohide && autohide_manager){
        delete autohide_manager; // expands the panel
        autohide_manager = nullptr;
    }
    if (autohide_manager)
        autohide_manager->set_delay(settings->value(panelconfig::autohide_delay, panelconfig::autohide_delay_default).toInt());

    QString position = settings->value(panelconfig::position, panelconfig::position_default).toString().toLower();
    geometry_manager->set_panel_position(position);
    geometry_manager->update_geometry();
}

void panel::loadplugins(){
    int sindex = 1;//for styling individual separators
    settings->beginGroup("plugins");
    foreach(QString key, settings->childGroups()){
        if (settings->value(key+"/enabled", false).toBool()){
            QString path = settings->value(key+"/path").toString();
            if (path == "separator"){
                QFrame *sepframe = new QFrame;
                sepframe->setObjectName("panelSeparator");
                sepframe->setProperty("SeparatorIndex", sindex);
                wlayout->addWidget(sepframe);
                sindex++;
                pluglist.append(nullptr);
            }
            else {
                addplugin(path);
            }
        }
    }
    settings->endGroup();

    if (numofstretchplugs == 0)
        wlayout->addStretch(1);
}

void panel::reloadplugins(){
    settings->sync();
    foreach(panelpluginterface *plug, pluglist){
        if (plug) plug->closePlug();
    }

    pluglist.clear();
    settings_plugs.clear();
    numofstretchplugs = 0;

    QLayoutItem *child;
    while ((child = wlayout->takeAt(0)) != nullptr) {
        delete child->widget(); // delete the widget
        delete child;   // delete the layout item
    }

    loadplugins();
}

void panel::addplugin(QString path){
    QPluginLoader *plugloader = new QPluginLoader(path);
    if (plugloader->load()){
        QObject *plugin = plugloader->instance();
        if (plugin){
            pluginterface = qobject_cast<panelpluginterface *>(plugin);
            if (pluginterface){
                pluglist.append(pluginterface);

                QJsonObject info = plugloader->metaData().value("MetaData").toObject();
                bool stretch = info.value("stretch").toBool();
                if (stretch)
                    numofstretchplugs++;

                if (!settingsaction) {
                    settingsaction = new QAction(QIcon::fromTheme("preferences-system"), "Panel Settings", this);
                    connect(settingsaction, &QAction::triggered, this, &panel::showsettings);
                    XdgActivation::instance()->watch(settingsaction);
                }
                QList<QAction*> actions = {settingsaction};
                QString settings_path = info.value("settings").toString();
                if (!settings_path.isEmpty()) {
                    settings_plugs[settings_path] = pluginterface;
                    // Parented to the applet so it goes when the applet does.
                    QAction *appletaction = new QAction(QIcon::fromTheme("configure"), info.value("name").toString() + " Settings", plugin);
                    connect(appletaction, &QAction::triggered, this, [settings_path]{
                        XdgActivation::instance()->launch("forest-settings", {settings_path});
                    });
                    XdgActivation::instance()->watch(appletaction);
                    QAction *separator = new QAction(plugin);
                    separator->setSeparator(true);
                    actions << separator << appletaction;
                }
                pluginterface->setupPlug(wlayout, actions);

                if (stretch)
                    wlayout->setStretch(wlayout->count()-1, 5);
            }
        }
    }
    else { qDebug() << plugloader->errorString(); }
}

void panel::reloadappletsettings(const QString &settings_path){
    if (panelpluginterface *plug = settings_plugs.value(settings_path))
        plug->reloadSettings();
}

void panel::update_panel_size() {
    // Hacky way to make the panel size change when the theme changes
    if (geometry_manager && pframe) {
        geometry_manager->set_fixed_size(pframe->height());
        geometry_manager->update_geometry();
    }
}
