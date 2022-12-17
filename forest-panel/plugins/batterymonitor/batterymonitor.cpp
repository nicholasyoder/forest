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

#include "batterymonitor.h"

batterymonitor::batterymonitor()
{

}

batterymonitor::~batterymonitor()
{
}

void batterymonitor::setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist)
{
    this->setLayout(basehlayout);
    basehlayout->setMargin(4);
    basehlayout->setSpacing(0);

    panelbutton *pb = new panelbutton;
    //pb->setbuttondata(this);
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setMargin(0);
    hlayout->addWidget(this);
    pb->setLayout(hlayout);
    layout->addWidget(pb);

    QDir psdir(PATH_TO_PS_DIR);
    foreach (QString sdir, psdir.entryList())
    {
        if (sdir.startsWith("bat", Qt::CaseInsensitive))
        {
            battery *bat = new battery(PATH_TO_PS_DIR + sdir);
            batterylist.append(bat);
            basehlayout->addWidget(bat);
        }
    }

    if (batterylist.count() == 0)
    {
        battery *bat = new battery("");
        basehlayout->addWidget(bat);
        bat->updatedata();
    }
    else
    {
        connect(updatetimer, SIGNAL(timeout()), this, SLOT(updatedata()));
        updatetimer->start(3000);
    }

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(popuplabel);
    popuplabel->setForegroundRole(QPalette::Light);
    pbox = new popup(vlayout, this, CenteredOnWidget);
    connect(pb, &panelbutton::leftclicked, this, &batterymonitor::showpopup);
}

QHash<QString, QString> batterymonitor::getpluginfo()
{
    QHash<QString, QString> info;
    info["name"] = "Battery Monitor";
    return info;
}


void batterymonitor::updatedata()
{
    foreach (battery *bat, batterylist)
    {
        bat->updatedata();
    }
}

void batterymonitor::showpopup()
{
    QString popuptext;
    int c = 0;
    foreach (battery *bat, batterylist)
    {
        popuptext.append("Battery " + QString::number(c) + "\n");
        popuptext.append(bat->getstatus() + "\n");
        popuptext.append(QString::number(qRound(bat->getpercentfull()*100)) + "% full");
        c++;
    }
    popuplabel->setText(popuptext);
    pbox->showpopup();
}
