// SPDX-License-Identifier: LGPL-3.0-or-later

#include "quicklaunch.h"
#include "xdgactivation.h"

quicklaunch::quicklaunch(QWidget *parent) : QWidget(parent){}
quicklaunch::~quicklaunch(){ delete pmenu; }

void quicklaunch::setupPlug(QBoxLayout *layout, QList<QAction*> itemlist){
    XdgActivation::instance(); // binds asynchronously; must be ready by the first launch
    basehlayout->setContentsMargins(QMargins(0,0,0,0));
    basehlayout->setSpacing(0);
    this->setLayout(basehlayout);
    this->setAcceptDrops(true);

    pmenu = new QMenu;
    pmenu->addActions(itemlist);

    pmenu->addSeparator();
    pmenu->addAction(QIcon::fromTheme("remove"), "Remove Launcher", this, &quicklaunch::removelauncher);

    parentlayout = layout;

    settings = new QSettings("Forest","Quicklaunch");

    layout->addWidget(this);
    basehlayout->setDirection(layout->direction());

    loadlaunchers();

    QDBusConnection::sessionBus().registerObject("/org/forest/panel/quicklaunch", this, QDBusConnection::ExportAllSlots);
}

void quicklaunch::dragEnterEvent(QDragEnterEvent *event){
    if (event->mimeData()->hasUrls()){
        bool hasurl = true;

        foreach (QUrl url, event->mimeData()->urls()){
            if (!url.isLocalFile() || !url.toLocalFile().endsWith(".desktop"))
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
            if (url.isLocalFile())
                settings->setValue("launchers/launch-" + padwithzeros(nextkeynumber()), url.toLocalFile());
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
    settings->setValue("launchers/launch-"+padwithzeros(nextkeynumber()), desktopfilepath);
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
    popupMenuOnLauncher(pmenu, launcherlist.at(currentlauncher), EdgeAlignedOnWidget);
}

void quicklaunch::removelauncher(){
    settings->remove("launchers");
    launcher *l = launcherlist.at(currentlauncher);
    l->close();
    launcherlist.removeAt(currentlauncher);
    delete l;

    savelaunchermove();
}

// One past the highest stored key, so it never overwrites an existing entry.
int quicklaunch::nextkeynumber(){
    settings->beginGroup("launchers");
    int next = 0;
    foreach (QString key, settings->childKeys())
        next = qMax(next, key.mid(key.lastIndexOf('-') + 1).toInt() + 1);
    settings->endGroup();
    return next;
}

QString quicklaunch::padwithzeros(int number){
    if (number < 10) return "000" + QString::number(number);
    else if (number < 100) return "00" + QString::number(number);
    else if (number < 1000) return "0" + QString::number(number);
    else return QString::number(number);
}
