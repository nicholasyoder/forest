// SPDX-License-Identifier: LGPL-3.0-or-later

#include "panel.h"

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
    QProcess::startDetached("forest-settings", QStringList("desktop/panel"));
}

void panel::loadsettings(){
    bool autohide = settings->value("autohide").toBool();

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
        autohide_manager->set_delay(settings->value("autohide_delay", 1000).toInt());

    QString position = settings->value("position", "bottom").toString().toLower();
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

                QHash<QString, QString> info = pluginterface->getpluginfo();
                bool stretch = false;
                if (info["stretch"] == "true"){
                    stretch = true;
                    numofstretchplugs++;
                }

                if (!settingsaction) {
                    settingsaction = new QAction(QIcon::fromTheme("preferences-system"), "Panel Settings", this);
                    connect(settingsaction, &QAction::triggered, this, &panel::showsettings);
                }
                pluginterface->setupPlug(wlayout, {settingsaction});

                if (stretch)
                    wlayout->setStretch(wlayout->count()-1, 5);
            }
        }
    }
    else { qDebug() << plugloader->errorString(); }
}

void panel::update_panel_size() {
    // Hacky way to make the panel size change when the theme changes
    if (geometry_manager && pframe) {
        geometry_manager->set_fixed_size(pframe->height());
        geometry_manager->update_geometry();
    }
}
