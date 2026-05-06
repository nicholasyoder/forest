// SPDX-License-Identifier: LGPL-3.0-or-later

#include "clock.h"
#include "clocksettingswidget.h"

clockplug::clockplug(){}

clockplug::~clockplug(){}

void clockplug::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist){
    setupTextButton("12:00");

    QTimer *uptimer = new QTimer;
    connect(uptimer, SIGNAL(timeout()), this, SLOT(updatetime()));
    uptimer->start(1000);

    QVBoxLayout *popuplayout = new QVBoxLayout;
    popuplayout->addWidget(cwidget);
    popupbox = new popup(popuplayout, this, CenteredOnWidget);

    pmenu = new popupmenu(this, CenteredOnWidget);
    foreach (pmenuitem *item, itemlist)
        pmenu->additem(item);

    pmenu->addseperator();
    pmenuitem *item = new pmenuitem("Clock Settings", QIcon::fromTheme("configure"));
    connect(item, &pmenuitem::clicked, this, &clockplug::showsettingswidget);
    pmenu->additem(item);

    layout->addWidget(this);

    loadsettings();

    connect(this, &clockplug::leftclicked, this, &clockplug::showpopup);
    connect(this, &clockplug::rightclicked, pmenu, &popupmenu::show);
}

QHash<QString, QString> clockplug::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "Clock";
    return info;
}

void clockplug::loadsettings(){
    QSettings settings("Forest", "Clock");
    settings.sync();
    twelvehour = settings.value("12hour", true).toBool();
    showseconds = settings.value("showseconds", false).toBool();
    time_format = "h:mm";
    if(showseconds)
        time_format += ":ss";
    if (twelvehour)
        time_format += " AP";
    updatetime();
}

void clockplug::updatetime(){
    QDateTime now = QDateTime::currentDateTime();
    setText(now.time().toString(time_format));

    // Also update calendar widget when the day changes
    QDate date = now.date();
    if (date != currentDate) {
        currentDate = date;
        cwidget->setSelectedDate(date);
    }
}

void clockplug::showsettingswidget(){
    clocksettingswidget *settingsw = new clocksettingswidget;
    settingsw->setWindowFlags(Qt::Dialog);
    connect(settingsw, SIGNAL(settingschanged()), this, SLOT(loadsettings()));
    settingsw->show();
}
