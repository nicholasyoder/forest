// SPDX-License-Identifier: LGPL-3.0-or-later

#include "clock.h"
#include "clockconfig.h"

clockplug::clockplug() : panelbutton(Text) {}

clockplug::~clockplug(){ delete pmenu; delete popupbox; }

void clockplug::setupPlug(QBoxLayout *layout, QList<QAction*> itemlist){
    setText("12:00");

    uptimer = new QTimer(this);
    uptimer->setSingleShot(true);
    connect(uptimer, &QTimer::timeout, this, &clockplug::updatetime);

    QVBoxLayout *popuplayout = new QVBoxLayout;
    popuplayout->addWidget(cwidget);
    popupbox = new popup(popuplayout, this, CenteredOnWidget);

    pmenu = new QMenu;
    pmenu->addActions(itemlist);

    layout->addWidget(this);

    loadsettings();

    connect(this, &clockplug::leftclicked, this, &clockplug::showpopup);
    connect(this, &clockplug::rightclicked, this, [this]{ popupMenuOnLauncher(pmenu, this, CenteredOnWidget); });
}

void clockplug::loadsettings(){
    QSettings settings("Forest", "Clock");
    settings.sync();
    twelvehour = settings.value(clockconfig::twelvehour, clockconfig::twelvehour_default).toBool();
    showseconds = settings.value(clockconfig::showseconds, clockconfig::showseconds_default).toBool();
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

    // Fire on the next second/minute boundary instead of polling at 1 Hz.
    const QTime t = now.time();
    uptimer->start(showseconds ? 1000 - t.msec() : (60 - t.second()) * 1000 - t.msec());

    // Also update calendar widget when the day changes
    QDate date = now.date();
    if (date != currentDate) {
        currentDate = date;
        cwidget->setSelectedDate(date);
    }
}
