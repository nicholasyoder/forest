/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "clock.h"

clockplug::clockplug()
{

}

clockplug::~clockplug()
{
}

void clockplug::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist)
{
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

QHash<QString, QString> clockplug::getpluginfo()
{
    QHash<QString, QString> info;
    info["name"] = "Clock";
    return info;
}

void clockplug::loadsettings()
{
    QSettings settings("Forest", "Clock");
    settings.sync();
    twelvehour = settings.value("12hour", true).toBool();
    showseconds = settings.value("showseconds", false).toBool();
    updatetime();
}

void clockplug::updatetime()
{
    if (twelvehour)
    {
        if (showseconds)
            setText(QDateTime::currentDateTime().time().toString("h:mm:ss AP"));
        else
            setText(QDateTime::currentDateTime().time().toString("h:mm AP"));
    }
    else
    {
        if (showseconds)
            setText(QDateTime::currentDateTime().time().toString("h:mm:ss"));
        else
            setText(QDateTime::currentDateTime().time().toString("h:mm"));
    }

    /*if (baselayout->direction() == QBoxLayout::RightToLeft)
        this->setFixedHeight(timelabel->minimumSizeHint().height() + baselayout->margin()*2);
    else
        this->setFixedWidth(timelabel->minimumSizeHint().width() + baselayout->margin()*2);
*/
    /*if (this->size() != widgetsize)
    {
        widgetsize = this->size();
        parentlayout->update();
    }*/
}

void clockplug::showsettingswidget()
{
    clocksettingswidget *settingsw = new clocksettingswidget;
    settingsw->setWindowFlags(Qt::Dialog);
    connect(settingsw, SIGNAL(settingschanged()), this, SLOT(loadsettings()));
    settingsw->show();
}
