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

#include "quicklaunch.h"

quicklaunch::quicklaunch(QWidget *parent) : QWidget(parent){

}

quicklaunch::~quicklaunch(){

}

void quicklaunch::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist){
    basehlayout->setMargin(0);
    basehlayout->setSpacing(0);
    this->setLayout(basehlayout);
    this->setAcceptDrops(true);

    pmenu = new popupmenu(this, EdgeAlignedOnWidget);
    foreach (pmenuitem *item, itemlist)
        pmenu->additem(item);

    pmenu->addseperator();
    /*pmenuitem *item = new pmenuitem("Add Launcher", QIcon::fromTheme("list-add"));
    connect(item, &QPushButton::clicked, this, &quicklaunch::showcustomlaunchw);
    pmenu->additem(item);*/
    pmenuitem *item2 = new pmenuitem("Remove Launcher", QIcon::fromTheme("remove"));
    connect(item2, &QPushButton::clicked, this, &quicklaunch::removelauncher);
    pmenu->additem(item2);

    parentlayout = layout;

    settings = new QSettings("Forest","Quicklaunch");

    layout->addWidget(this);
    basehlayout->setDirection(layout->direction());

    loadlaunchers();

    QDBusConnection::sessionBus().registerObject("/org/forest/panel/quicklaunch", this, QDBusConnection::ExportAllSlots);
}

QHash<QString, QString> quicklaunch::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "Quicklaunch";
    return info;
}

void quicklaunch::dragEnterEvent(QDragEnterEvent *event){
    if (event->mimeData()->hasUrls()){
        bool hasurl = true;

        foreach (QUrl url, event->mimeData()->urls()){
            if (!url.toString().endsWith(".desktop"))
                hasurl = false;
        }
        if (hasurl)
            event->acceptProposedAction();
    }
}

void quicklaunch::dropEvent(QDropEvent *event){
    if (event->mimeData()->hasUrls()){
        QList<QUrl> urls = event->mimeData()->urls();
        foreach (QUrl url, urls){
            QString filepath;
            filepath = url.toString();
            filepath.remove("file://");

            int num = launcherlist.count();
            settings->setValue("launchers/launch-" + padwithzeros(num), filepath);
        }

        event->acceptProposedAction();
        reloadlaunchers();
    }
}


void quicklaunch::reloadlaunchers(){
    launcherlist.clear();

    QLayoutItem *child;
    while ((child = basehlayout->takeAt(0)) != nullptr){
        delete child->widget();
        delete child;
    }
    loadlaunchers();
}

void quicklaunch::addlauncher(QString desktopfilepath){
    settings->setValue("launchers/launch-"+padwithzeros(launcherlist.count()), desktopfilepath);
    reloadlaunchers();
}

void quicklaunch::movelauncher(launcher *l, bool up){
    int index = basehlayout->indexOf(l);
    int listindex = launcherlist.indexOf(l);

    if (up){
        if (index != 0){
            QLayoutItem *item = basehlayout->takeAt(index);
            basehlayout->insertWidget(index-1, item->widget());
            launcherlist.move(listindex, listindex-1);
        }
    }
    else{
        if (index != basehlayout->count() - 1){
            QLayoutItem *item = basehlayout->takeAt(index);
            basehlayout->insertWidget(index+1, item->widget());
            launcherlist.move(listindex, listindex+1);
        }
    }
}

void quicklaunch::savelaunchermove(){
    settings->sync();
    for (int c = 0; c < launcherlist.count(); c++){
        launcherlist.at(c)->lnum = c;
        settings->setValue("launchers/launch-" + padwithzeros(c), launcherlist.at(c)->dfilepath);
    }
}

void quicklaunch::loadlaunchers(){
    settings->sync();
    settings->beginGroup("launchers");

    int c = 0;
    foreach(QString key, settings->childKeys()){
        launcher *launch = new launcher(c, settings->value(key).toString());//, menuitemlist);
        connect(launch, SIGNAL(moved(launcher*, bool)), this, SLOT(movelauncher(launcher*, bool)));
        connect(launch, SIGNAL(movefinished()), this, SLOT(savelaunchermove()));
        connect(launch, &launcher::rightclicked, this, &quicklaunch::showpopupmenu);
        basehlayout->addWidget(launch);
        launcherlist.append(launch);

        c++;
    }
    settings->endGroup();
}

void quicklaunch::showpopupmenu(int launchernum){
    currentlauncher = launchernum;
    pmenu->changelauncher(launcherlist.at(currentlauncher));
    pmenu->show();
}

void quicklaunch::removelauncher(){
    settings->remove("launchers");
    launcher *l = launcherlist.at(currentlauncher);
    l->close();
    launcherlist.removeAt(currentlauncher);
    delete l;

    savelaunchermove();
}

QString quicklaunch::padwithzeros(int number){
    if (number < 10) return "000" + QString::number(number);
    else if (number < 100) return "00" + QString::number(number);
    else if (number < 1000) return "0" + QString::number(number);
    else return QString::number(number);
}
