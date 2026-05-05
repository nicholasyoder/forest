// SPDX-License-Identifier: LGPL-3.0-or-later

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
    basehlayout->setContentsMargins(QMargins(0,0,0,0));
    basehlayout->setSpacing(0);

    panelbutton *pb = new panelbutton;
    //pb->setbuttondata(this);
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(QMargins(0,0,0,0));
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
