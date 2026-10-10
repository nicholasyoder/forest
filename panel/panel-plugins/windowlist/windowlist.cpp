// SPDX-License-Identifier: LGPL-3.0-or-later

#include "windowlist.h"

windowlist::windowlist(){}

windowlist::~windowlist(){
    delete pmenu;
}

void windowlist::setupPlug(QBoxLayout *layout, QList<QAction*> itemlist){
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

    pmenu = new QMenu;
    pmenu->addActions(itemlist);

    pmenu->addSeparator();
    pmenu->addAction(QIcon::fromTheme("configure"), "Windowlist Settings", this, &windowlist::showsettingswidget);

    ipopup = new imagepopup(this);

    loadsettings();

    tracker = new ToplevelTracker(this);
    connect(tracker, &ToplevelTracker::workspaceMembershipChanged, this, &windowlist::refreshVisibility);
    connect(tracker, &ToplevelTracker::toplevelAdded, this, &windowlist::onWindowAdded);
}

void windowlist::reloadsettings(){
    loadsettings();
    foreach (windowbutton *wbt, button_list) {
        wbt->setMaximumWidth(maxbtsize);
    }
}

void windowlist::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() == Qt::RightButton)
        popupMenuOnLauncher(pmenu, this, CenteredOnMouse);
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
    connect(handle, &ForeignToplevelHandle::changed, this, &windowlist::onWindowChanged);
    connect(handle, &ForeignToplevelHandle::closed, this, &windowlist::onWindowRemoved);
}

// The button is created on the first `done`, once title/app_id are known.
void windowlist::onWindowChanged(ForeignToplevelHandle *handle){
    if (windowbutton *wbt = button_list.value(handle)){
        wbt->syncFromHandle();
        return;
    }

    windowbutton *wbt = new windowbutton(handle, tracker->workspaceManager(), tracker->biomeWorkspaces());
    connect(wbt, &windowbutton::moved, this, &windowlist::onButtonMoved);
    connect(wbt, &windowbutton::mouseEnter, ipopup, &imagepopup::btmouseEnter);
    connect(wbt, &windowbutton::mouseLeave, ipopup, &imagepopup::btmouseLeave);
    connect(wbt, &windowbutton::request_ipopup_close, ipopup, &imagepopup::closepopup);
    wbt->setMaximumWidth(maxbtsize);
    mainlayout->addWidget(wbt, 1);
    button_list[handle] = wbt;

    refreshVisibility();
}

void windowlist::onWindowRemoved(ForeignToplevelHandle *handle){
    if (windowbutton *wbt = button_list.take(handle)){
        mainlayout->removeWidget(wbt);
        wbt->close();
        wbt->deleteLater();
    }
}

void windowlist::refreshVisibility(){
    foreach (windowbutton *wbt, button_list)
        wbt->setVisible(tracker->isOnActiveWorkspace(wbt->toplevelHandle()));
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
