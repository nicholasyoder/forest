// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CLOCK_H
#define CLOCK_H

#include <QWidget>
#include <QMenu>
#include <QLabel>
#include <QVBoxLayout>
#include <QDateTime>
#include <QCalendarWidget>
#include <QTimer>
#include <QDebug>
#include <QtDBus>

#include "panelpluginterface.h"
#include "panelbutton.h"
#include "popup.h"

class clockplug : public panelbutton, panelpluginterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.clock.plugin" FILE "clock.json")
    Q_INTERFACES(panelpluginterface)

public:
    clockplug();
    ~clockplug();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<QAction*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    void reloadSettings(){ loadsettings(); }
    //end plugininterface

private slots:
    void loadsettings();
    void updatetime();
    void showpopup(){popupbox->showpopup();}

private:
    bool twelvehour = false;
    bool showseconds = false;
    QString time_format = "h:mm";
    QDate currentDate;
    QCalendarWidget *cwidget = new QCalendarWidget;
    popup *popupbox;
    QMenu *pmenu = nullptr;
};

#endif // CLOCK_H
