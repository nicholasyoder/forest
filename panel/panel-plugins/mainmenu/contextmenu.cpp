#include "contextmenu.h"
#include "propertieswidget.h"

#include <QMenu>
#include <QtDBus>
#include <qt5xdg/XdgIcon>

contextmenu::contextmenu()
{

}

void contextmenu::show(XdgDesktopFile deskfile, QPoint pos){
    currentDeskFile = deskfile;

    QMenu *cmenu = new QMenu;
    QAction *qlaunch = new QAction(XdgIcon::fromTheme("list-add"), "Add to quicklaunch");
    connect(qlaunch, &QAction::triggered, this, &contextmenu::add2Quicklaunch);
    cmenu->addAction(qlaunch);
    QAction *desktop = new QAction(XdgIcon::fromTheme("user-desktop"), "Show on desktop");
    connect(desktop, &QAction::triggered, this, &contextmenu::showOnDesktop);
    cmenu->addAction(desktop);
    //QAction *root = new QAction(XdgIcon::fromTheme("application-x-executable"), "Run as root");
    //connect(root, &QAction::triggered, this, &contextmenu::runAsRoot);
    //cmenu->addAction(root);
    QAction *prop = new QAction(XdgIcon::fromTheme("edit-entry"), "Properties");
    connect(prop, &QAction::triggered, this, &contextmenu::showProperties);
    cmenu->addAction(prop);
    cmenu->popup(pos);
}

void contextmenu::add2Quicklaunch(){
    if (QDBusConnection::sessionBus().isConnected()){
        QDBusInterface iface("org.forest", "/org/forest/panel/quicklaunch", "", QDBusConnection::sessionBus());
        if (iface.isValid())
            iface.call("addlauncher", currentDeskFile.fileName());
        else
            fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }
    else{
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\nTo start it, run:\n\teval `dbus-launch --auto-syntax`\n");
    }
}

void contextmenu::showOnDesktop(){
    QFile::copy(currentDeskFile.fileName(), QDir::homePath() + "/Desktop/" + currentDeskFile.fileName().split("/").last());
}

void contextmenu::runAsRoot(){

}

void contextmenu::showProperties(){
    if (currentDeskFile.isValid()){
        propertieswidget *pwidget = new propertieswidget;
        pwidget->setWindowFlags(Qt::Dialog);
        pwidget->setdata(currentDeskFile.name(),
                         currentDeskFile.localizedValue("Exec").toString(),
                         currentDeskFile.comment(),
                         currentDeskFile.iconName(),
                         currentDeskFile.fileName());
        pwidget->show();
    }
}
