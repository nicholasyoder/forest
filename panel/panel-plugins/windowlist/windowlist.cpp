// SPDX-License-Identifier: LGPL-3.0-or-later

#include "windowlist.h"

#include "iconresolver.h"

windowlist::windowlist(){}

windowlist::~windowlist(){}

void windowlist::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist){
    layout->addWidget(this);

    QHBoxLayout *baseLayout = new QHBoxLayout;
    baseLayout->addLayout(mainlayout);
    baseLayout->setContentsMargins(QMargins(0,0,0,0));
    baseLayout->setSpacing(0);
    mainlayout->setContentsMargins(QMargins(0,0,0,0));
    mainlayout->setSpacing(0);
    QWidget *swidget = new QWidget;
    baseLayout->addWidget(swidget);
    this->setLayout(baseLayout);

    pmenu = new popupmenu(this, CenteredOnMouse);
    foreach (pmenuitem *item, itemlist)
        pmenu->additem(item);

    pmenu->addseperator();
    pmenuitem *item = new pmenuitem("Windowlist Settings", QIcon::fromTheme("configure"));
    connect(item, &pmenuitem::clicked, this, &windowlist::showsettingswidget);
    pmenu->additem(item);

    ipopup = new imagepopup(this);

    loadsettings();

    // No manual seeding step needed here: binding the manager makes the
    // compositor replay a `toplevel` event for every already-open window,
    // same as it does for windows opened afterward.
    toplevel_manager = new ForeignToplevelManager();
    connect(toplevel_manager, &ForeignToplevelManager::toplevelCreated, this, &windowlist::onWindowAdded);
}

QHash<QString, QString> windowlist::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "Window List";
    info["stretch"] = "true";
    return info;
}

void windowlist::reloadsettings(){
    loadsettings();
    foreach (windowbutton *wbt, button_list) {
        wbt->setMaximumWidth(maxbtsize);
    }
}

void windowlist::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() == Qt::RightButton)
        pmenu->show();
}

void windowlist::loadsettings(){
    QSettings settings("Forest", "Window List");
    settings.sync();
    ipopup->set_enabled(settings.value("showthumbnails", true).toBool());
    maxbtsize = settings.value("maxbuttonsize", 170).toInt();
}

void windowlist::showsettingswidget(){
    connect(swidget, SIGNAL(settingschanged()), this, SLOT(reloadsettings()));
    swidget->show();
}


void windowlist::onWindowAdded(ForeignToplevelHandle *handle){
    // No filtering: wlr-foreign-toplevel-management has no window-type/
    // skip-taskbar concept, so unlike the old KWindowInfo-based
    // acceptWindow(), every toplevel the compositor reports gets a button.
    if (button_list.contains(handle))
        return;

    connect(handle, &ForeignToplevelHandle::changed, this, &windowlist::onWindowChanged);
    connect(handle, &ForeignToplevelHandle::closed, this, &windowlist::onWindowRemoved);

    windowbutton *wbt = new windowbutton(handle, iconresolver::iconForAppId(handle->appId()), handle->title());
    connect(wbt, &windowbutton::moved, this, &windowlist::onButtonMoved);
    connect(wbt, &windowbutton::mouseEnter, ipopup, &imagepopup::btmouseEnter);
    connect(wbt, &windowbutton::mouseLeave, ipopup, &imagepopup::btmouseLeave);
    connect(wbt, &windowbutton::request_ipopup_close, ipopup, &imagepopup::closepopup);
    wbt->setMaximumWidth(maxbtsize);
    mainlayout->addWidget(wbt, 1);
    button_list[handle] = wbt;
}

void windowlist::onWindowRemoved(ForeignToplevelHandle *handle){
    if(!button_list.contains(handle))
        return;

    windowbutton *wbt = button_list[handle];
    button_list.remove(handle);
    mainlayout->removeWidget(wbt);
    wbt->close();
    wbt->deleteLater();
    handle->deleteLater();
}

void windowlist::onWindowChanged(ForeignToplevelHandle *handle){
    if(!button_list.contains(handle))
        return;

    windowbutton *wbt = button_list[handle];
    wbt->setText(handle->title());
    wbt->setIcon(iconresolver::iconForAppId(handle->appId()));
    // Each handle already knows whether it itself is active, so no
    // separate "current active window" query/tracking is needed here the
    // way Xcbutills::getActiveWindow() required.
    wbt->setDown(handle->isActivated());
}

void windowlist::onButtonMoved(windowbutton *wbt, bool left){
    int index = mainlayout->indexOf(wbt);

    if (left){
        if (index != 0){
            QLayoutItem *item = mainlayout->takeAt(index);
            mainlayout->insertWidget(index-1, item->widget(), 1);
        }
    }
    else{
        if (index != mainlayout->count() - 1){
            QLayoutItem *item = mainlayout->takeAt(index);
            mainlayout->insertWidget(index+1, item->widget(), 1);
        }
    }
}
