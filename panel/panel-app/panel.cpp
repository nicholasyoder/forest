/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021-2024 Nicholas Yoder
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

#include "panel.h"
#include "hiddenpanel.h"

panel::panel(){}

panel::~panel(){}

void panel::setupPlug(){
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    setWindowFlags(Qt::FramelessWindowHint);

    geometry_manager = new GeometryManager(this);

    wlayout = new QHBoxLayout;
    wlayout->setMargin(0);
    wlayout->setSpacing(0);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setMargin(0);
    pframe = new panelQFrame();
    pframe->setObjectName("panel");
    pframe->setLayout(wlayout);
    vlayout->addWidget(pframe);
    connect(pframe, &panelQFrame::resized, this, &panel::update_panel_size);

    loadsettings();
    loadplugins();

    QDBusConnection::sessionBus().registerObject("/org/forest/panel", this, QDBusConnection::ExportAllSlots);

    show();
}

void panel::XcbEventFilter(xcb_generic_event_t *event){
    foreach (panelpluginterface *interface, xcbpluglist) {
        interface->XcbEventFilter(event);
    }
}

void panel::showsettings(){
    QProcess::startDetached("forest-settings", QStringList("Panel"));
}

void panel::loadsettings(){
    bool autohide = settings->value("autohide").toBool();

    // only reserve screen space for panel if it is always visible
    geometry_manager->set_reserve_screen_space(!autohide);

    if (autohide && !autohide_manager){
        autohide_manager = new AutoHideManager(this);
        connect(autohide_manager, &AutoHideManager::show, this, &panel::show);
        connect(autohide_manager, &AutoHideManager::hide, this, &panel::hide);
        installEventFilter(autohide_manager);

        HiddenPanel* hidden_panel = new HiddenPanel;
        connect(autohide_manager, &AutoHideManager::show, hidden_panel, &HiddenPanel::hide);
        connect(autohide_manager, &AutoHideManager::hide, hidden_panel, &HiddenPanel::show);
        connect(hidden_panel, &HiddenPanel::activated, this, &panel::show);

        hp_geometry_manager = new GeometryManager(hidden_panel);
        hp_geometry_manager->set_fixed_size(1);

        if(this->isVisible()) // During settings reload
            autohide_manager->close_eventually(); // Start autohide process
        // During normal startup settings load, close_eventually will be
        // called by the show event filter in AutoHideManager
    }
    else if (!autohide && autohide_manager){
        autohide_manager->disconnect();
        delete autohide_manager;
        autohide_manager = nullptr;

        hp_geometry_manager->disconnect();
        delete hp_geometry_manager;
        hp_geometry_manager = nullptr;

        this->show();
    }

    QString position = settings->value("position", "bottom").toString().toLower();
    geometry_manager->set_panel_position(position);
    geometry_manager->update_geometry();
    if (hp_geometry_manager){
        hp_geometry_manager->set_panel_position(position);
        hp_geometry_manager->update_geometry();
    }
}

void panel::loadplugins(){
    int sindex = 1;//for styling individual seperators
    settings->beginGroup("plugins");
    foreach(QString key, settings->childGroups()){
        if (settings->value(key+"/enabled", false).toBool()){
            QString path = settings->value(key+"/path").toString();
            if (path == "seperator"){
                QFrame *sepframe = new QFrame;
                sepframe->setObjectName("panelSeparator");
                sepframe->setProperty("SeperatorIndex", sindex);
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
    foreach(panelpluginterface *plug, pluglist){
        if (plug) plug->closePlug();
    }

    xcbpluglist.clear();
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
                if (info["needsXcbEvents"] == "true")
                    xcbpluglist.append(pluginterface);

                bool stretch = false;
                if (info["stretch"] == "true"){
                    stretch = true;
                    numofstretchplugs++;
                }

                pmenuitem *settingsitem = new pmenuitem("Panel Settings", QIcon::fromTheme("preferences-system"));
                connect(settingsitem, &pmenuitem::clicked, this, &panel::showsettings);
                QList<pmenuitem*> itemlist;
                itemlist.append(settingsitem);

                pluginterface->setupPlug(wlayout, itemlist);

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
