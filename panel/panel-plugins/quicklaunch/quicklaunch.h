#ifndef QUICKLAUNCH_H
#define QUICKLAUNCH_H

#include <QWidget>
#include <QSettings>
#include <QHBoxLayout>
#include <QGenericPlugin>
#include <QtDBus>

#include "launcher.h"
#include "panelpluginterface.h"

class quicklaunch : public QWidget, panelpluginterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.quicklaunch.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    quicklaunch(QWidget *parent = nullptr);
    ~quicklaunch();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    void XcbEventFilter(xcb_generic_event_t* /*event*/){}
    QHash<QString, QString> getpluginfo();
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
    popupmenu *pmenu;
    int currentlauncher = 0;
    QSettings *settings;

};
#endif // QUICKLAUNCH_H
