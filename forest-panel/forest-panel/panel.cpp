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

#include "panel.h"

panel::panel(){}

panel::~panel(){}

void panel::setupPlug(){
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock);

    Qt::WindowFlags flags;
    flags |= Qt::WindowStaysOnTopHint;
    flags |= Qt::FramelessWindowHint;
    this->setWindowFlags(flags);

    panelposition = settings->value("position", "bottom").toString().toLower();
    panelsize = sizeHint().height();

    if (panelposition == "left" || panelposition == "right")
        wlayout = new QVBoxLayout;
    else
        wlayout = new QHBoxLayout;

    wlayout->setMargin(0);
    wlayout->setSpacing(0);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setMargin(0);
    panelQFrame *pframe = new panelQFrame;
    pframe->setObjectName("panel");
    pframe->setLayout(wlayout);
    vlayout->addWidget(pframe);
    connect(pframe, &panelQFrame::resized, this, &panel::resetgeometry);

    loadsettings();
    loadplugins();

    QDBusConnection::sessionBus().registerObject("/org/forest/panel", this, QDBusConnection::ExportAllSlots);

    connect(qApp->primaryScreen(), &QScreen::geometryChanged, this, &panel::reloadsettings);

    this->show();
}

void panel::XcbEventFilter(xcb_generic_event_t *event){
    foreach (panelpluginterface *interface, xcbpluglist) {
        interface->XcbEventFilter(event);
    }
}

QHash<QString, QString> panel::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "Panel";
    info["icon"] = "preferences-desktop";
    info["needsXcbEvents"] = "true";
    return info;
}

void panel::showsettings(){
    QProcess::startDetached("forest-settings", QStringList("Panel"));
}

void panel::loadsettings(){
    QSettings s("Forest", "Panel");
    panelposition = s.value("position").toString().toLower();

    this->setMinimumSize(0,0);
    this->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    resetgeometry();
}

void panel::resetgeometry(){
    panelsize = sizeHint().height();

    QSize scsize = qApp->primaryScreen()->size();
    if (panelposition == "top"){
        this->move(0,0);
        this->setFixedSize(scsize.width(), panelsize);
        Xcbutills::setPartialStrut(winId(),0,0,height(),0,0,0,0,0,geometry().left(),geometry().right(),0,0);
    } else if (panelposition == "left"){
        this->move(0,0);
        this->setFixedSize(panelsize, scsize.height());
        //Xcbutills::setPartialStrut(winId(),height(),0,0,0,geometry().top(),geometry().bottom(),0,0,0,0,0,0);
    } else if (panelposition == "right"){
        this->move(scsize.width() - panelsize,0);
        this->setFixedSize(panelsize, scsize.height());
        //Xcbutills::setPartialStrut(winId(),0,height(),0,0,0,0,geometry().top(),geometry().bottom(),0,0,0,0);
    } else{//bottom
        this->move(0,scsize.height() - panelsize);
        this->setFixedSize(scsize.width(), panelsize);
        Xcbutills::setPartialStrut(winId(),0,0,0,height(),0,0,0,0,0,0,geometry().left(),geometry().right());
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

/*void panel::movepluginup(int plugnum)
{
    QLayoutItem *currentitem = wlayout->itemAt(plugnum);
    wlayout->removeItem(currentitem);
    if (currentitem->widget())
        wlayout->insertWidget(plugnum -1, currentitem->widget());
    else if (currentitem->layout())
        wlayout->insertLayout(plugnum -1, currentitem->layout());
    else if (currentitem->spacerItem())
        wlayout->insertSpacerItem(plugnum -1, currentitem->spacerItem());

    pluglist.move(plugnum, plugnum -1);

    this->update();
}

void panel::moveplugindown(int plugnum)
{
    QLayoutItem *currentitem = wlayout->itemAt(plugnum);
    wlayout->removeItem(currentitem);
    if (currentitem->widget())
        wlayout->insertWidget(plugnum +1, currentitem->widget());
    else if (currentitem->layout())
        wlayout->insertLayout(plugnum +1, currentitem->layout());
    else if (currentitem->spacerItem())
        wlayout->insertSpacerItem(plugnum +1, currentitem->spacerItem());

    pluglist.move(plugnum, plugnum +1);

    this->update();
}*/

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

/*void panel::addseperator(int plugnum)
{
    settings->beginGroup("seperators");

    int size = settings->value("seperator" + QString::number(plugnum) + "size").toInt();
    QSpacerItem *item = new QSpacerItem(size, size, QSizePolicy::Fixed, QSizePolicy::Fixed);
    wlayout->insertSpacerItem(plugnum, item);

    settings->endGroup();

    this->update();
}

void panel::setseperatorsettings(int plugnum, int size)
{
    QLayoutItem *item = wlayout->itemAt(plugnum);
    if (item->spacerItem())
        item->spacerItem()->changeSize(size,size, QSizePolicy::Fixed, QSizePolicy::Fixed);

    this->update();
}

void panel::removeplugin(int num)
{
    panelpluginterface *plug = pluglist.at(num);
    pluglist.removeAt(num);

    if (xcbpluglist.contains(plug))
        xcbpluglist.removeOne(plug);

    plug->closePlug();
}*/
