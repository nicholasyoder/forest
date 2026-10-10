// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QUICKLAUNCH_H
#define QUICKLAUNCH_H

#include <QWidget>
#include <QMenu>
#include <QSettings>
#include <QHBoxLayout>
#include <QGenericPlugin>
#include <QtDBus>

#include "launcher.h"
#include "panelpluginterface.h"
#include "panelanchor.h"

class quicklaunch : public QWidget, panelpluginterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.quicklaunch.plugin" FILE "quicklaunch.json")
    Q_INTERFACES(panelpluginterface)

public:
    quicklaunch(QWidget *parent = nullptr);
    ~quicklaunch();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<QAction*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    //end plugininterface

public slots:
    void reloadlaunchers();
    void addlauncher(QString desktopfilepath);
    void movelauncher(launcher *l, bool up);
    void savelaunchermove();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void loadlaunchers();
    void showpopupmenu(int launchernum);
    void removelauncher();
    QString padwithzeros(int number);

private:
    QList<launcher*> launcherlist;
    QHBoxLayout *basehlayout = new QHBoxLayout;
    QBoxLayout *parentlayout;
    QMenu *pmenu = nullptr;
    int currentlauncher = 0;
    QSettings *settings;

};
#endif // QUICKLAUNCH_H
