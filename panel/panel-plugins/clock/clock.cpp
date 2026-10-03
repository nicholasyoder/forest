// SPDX-License-Identifier: LGPL-3.0-or-later

#include "clock.h"
#include "clocksettingswidget.h"

clockplug::clockplug() : panelbutton(Text) {}

clockplug::~clockplug(){ delete pmenu; }

void clockplug::setupPlug(QBoxLayout *layout, QList<QAction*> itemlist){
    setText("12:00");

    QTimer *uptimer = new QTimer;
    connect(uptimer, SIGNAL(timeout()), this, SLOT(updatetime()));
    uptimer->start(1000);

    QVBoxLayout *popuplayout = new QVBoxLayout;
    popuplayout->addWidget(cwidget);
    popupbox = new popup(popuplayout, this, CenteredOnWidget);

    pmenu = new QMenu;
    pmenu->addActions(itemlist);

    pmenu->addSeparator();
    pmenu->addAction(QIcon::fromTheme("configure"), "Clock Settings", this, &clockplug::showsettingswidget);

    layout->addWidget(this);

    loadsettings();

    connect(this, &clockplug::leftclicked, this, &clockplug::showpopup);
    connect(this, &clockplug::rightclicked, this, [this]{ popupMenuOnLauncher(pmenu, this, CenteredOnWidget); });
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
