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

#include "cpumon.h"
#include <QGenericPlugin>

cpumon::cpumon(){}

cpumon::~cpumon(){}

//beginning of plugin interface
void cpumon::setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist){
    gwidget = new graphwidget;
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setMargin(0);
    vlayout->addWidget(gwidget);
    this->setLayout(vlayout);

    layout->addWidget(this);

    //popup menu~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pmenu = new popupmenu(this, CenteredOnWidget);
    foreach (pmenuitem *item, itemlist)
    {
        pmenu->additem(item);
    }
    pmenu->addseperator();
    pmenuitem *item = new pmenuitem("CPU Monitor Settings", QIcon::fromTheme("configure"));
    connect(item, &pmenuitem::clicked, this, &cpumon::showsettingswidget);
    pmenu->additem(item);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    connect(this, &cpumon::leftclicked, this, &cpumon::runcommand);
    connect(this, &cpumon::rightclicked, pmenu, &popupmenu::show);

    loadsettings();
}

QHash<QString, QString> cpumon::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "CPU Monitor";
    return info;
}
//end of plugin interface

void cpumon::loadsettings(){
    settings->sync();

    //colors~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    QColor backcolor;
    QStringList sbackcolor = settings->value("backgroundcolor", "0,0,0").toString().split(",");
    backcolor.setRed(sbackcolor.at(0).toInt());
    backcolor.setGreen(sbackcolor.at(1).toInt());
    backcolor.setBlue(sbackcolor.at(2).toInt());

    QColor forecolor;
    QStringList sforecolor = settings->value("foregroundcolor", "0,255,0").toString().split(",");
    forecolor.setRed(sforecolor.at(0).toInt());
    forecolor.setGreen(sforecolor.at(1).toInt());
    forecolor.setBlue(sforecolor.at(2).toInt());

    QList<QColor> colorlist;
    colorlist << forecolor;
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    qreal backopacity = settings->value("backgroundopacity", 1).toDouble();
    qreal foreopacity = settings->value("foregroundopacity", 1).toDouble();
    QList<qreal> opacitylist;
    opacitylist << foreopacity;

    gwidget->setupgraphs(1, colorlist, opacitylist, backcolor, backopacity);

    clickedcommand = settings->value("command", "lxtask").toString();

    delete refreshtimer;
    refreshtimer = new QTimer;
    connect(refreshtimer, SIGNAL(timeout()), this, SLOT(updatecpu()));
    refreshtimer->start(settings->value("refreshrate", 500).toInt());

    gwidget->setFixedWidth(settings->value("width", 40).toInt());
}

void cpumon::showsettingswidget(){
    settingswidget *swidget = new settingswidget;
    connect(swidget, &settingswidget::colorschanged, this, &cpumon::reloadcolors);
    connect(swidget, &settingswidget::settingschanged, this, &cpumon::reloadsettings);
    connect(swidget, &settingswidget::backOpChanged, this, &cpumon::setbackop);
    connect(swidget, &settingswidget::foreOpChanged, this, &cpumon::setforeop);
    swidget->show();
}

void cpumon::reloadcolors(){
    QColor backcolor;
    QStringList sbackcolor = settings->value("backgroundcolor", "0,0,0").toString().split(",");
    backcolor.setRed(sbackcolor.at(0).toInt());
    backcolor.setGreen(sbackcolor.at(1).toInt());
    backcolor.setBlue(sbackcolor.at(2).toInt());

    QColor forecolor;
    QStringList sforecolor = settings->value("foregroundcolor", "0,255,0").toString().split(",");
    forecolor.setRed(sforecolor.at(0).toInt());
    forecolor.setGreen(sforecolor.at(1).toInt());
    forecolor.setBlue(sforecolor.at(2).toInt());

    QList<QColor> colorlist;
    colorlist << forecolor;

    gwidget->backcolor = backcolor;
    gwidget->colors = colorlist;
    gwidget->update();
}

void cpumon::setbackop(qreal opacity){
    gwidget->backopacity = opacity;
    gwidget->update();
}

void cpumon::setforeop(qreal opacity){
    gwidget->opacitys[0] = opacity;
    gwidget->update();
}

void cpumon::reloadsettings(){
    loadsettings();
}

void cpumon::runcommand(){
    if (clickedcommand != "")
        QProcess::startDetached(clickedcommand);
}

void cpumon::updatecpu(){
    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QString line1 = file.readLine();
    line1.remove("cpu");

    QStringList sections = line1.split(QChar(' '), Qt::SkipEmptyParts);

    unsigned long long total = 0;

    for (int c = 0; c < sections.size(); c++)
        total += sections[c].toULongLong();

    unsigned long long usage = 0;
    usage += sections[0].toULongLong();
    usage += sections[1].toULongLong();
    usage += sections[2].toULongLong();

    for (int c = 4; c < sections.size(); c++)
        usage += sections[c].toULongLong();

    double divisor = double(total - oldtotal);
    double newusage = double(usage - oldvalue) / divisor;

    QList<qreal> values;
    values << newusage;
    gwidget->updategraph(values);

    oldvalue = usage;
    oldtotal = total;
}
