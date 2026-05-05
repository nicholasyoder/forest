// SPDX-License-Identifier: LGPL-3.0-or-later

#include "memmon.h"
#include <QGenericPlugin>

using namespace miscutills;

memmon::memmon(){}
memmon::~memmon(){}

//beginning of plugin interface
void memmon::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist){
    gwidget = new graphwidget;
    settings = new QSettings("Forest", "Memory Monitor");

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(QMargins(0,0,0,0));
    vlayout->addWidget(gwidget);
    setLayout(vlayout);
    layout->addWidget(this);

    pmenu = new popupmenu(this, CenteredOnWidget);
    foreach (pmenuitem *item, itemlist)
        pmenu->additem(item);

    pmenu->addseperator();
    pmenuitem *item = new pmenuitem("Memory Monitor Settings", QIcon::fromTheme("configure"));
    connect(item, &pmenuitem::clicked, this, &memmon::showsettingswidget);
    pmenu->additem(item);

    connect(this, &memmon::leftclicked, this, &memmon::runcommand);
    connect(this, &memmon::rightclicked, pmenu, &popupmenu::show);

    loadsettings();
}

QHash<QString, QString> memmon::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "Memmory monitor";
    return info;
}
//end of plugin interface

void memmon::loadsettings(){
    settings->sync();

    QColor backcolor = string_to_color(settings->value("backgroundcolor", "0,0,0").toString());
    QColor ramcolor = string_to_color(settings->value("RAMcolor", "255,0,0").toString());
    QColor swapcolor = string_to_color(settings->value("Swapcolor", "100,0,0").toString());
    QList<QColor> colorlist = {ramcolor, swapcolor};
    qreal backopacity = settings->value("backgroundopacity", 1).toDouble();
    qreal ramopacity = settings->value("RAMopacity", 1).toDouble();
    qreal swapopacity = settings->value("Swapopacity", 1).toDouble();
    QList<qreal> opacitylist = {ramopacity, swapopacity};

    QString sbehavior = settings->value("swapbehavior", "combine").toString();
    if (sbehavior == "disabled"){
        swapbehavior = 0;
        gwidget->setupgraphs(1, colorlist, opacitylist, backcolor, backopacity);
    }
    else if (sbehavior == "combine"){
        swapbehavior = 1;
        gwidget->setupgraphs(1, colorlist, opacitylist, backcolor, backopacity);
    }
    else if (sbehavior == "showseperate"){
        swapbehavior = 2;
        gwidget->setupgraphs(2, colorlist, opacitylist, backcolor, backopacity);
    }

    clickedcommand = settings->value("command", "lxtask").toString();

    delete refreshtimer;
    refreshtimer = new QTimer;
    connect(refreshtimer, SIGNAL(timeout()), this, SLOT(updatemem()));
    refreshtimer->start(settings->value("refreshrate", 500).toInt());

    gwidget->setFixedWidth(settings->value("width", 40).toInt());
}

void memmon::showsettingswidget(){
    settingswidget *swidget = new settingswidget;
    connect(swidget, SIGNAL(colorschanged()), this, SLOT(reloadcolors()));
    connect(swidget, SIGNAL(settingschanged()), this, SLOT(reloadsettings()));
    connect(swidget, SIGNAL(backOpChanged(qreal)), this, SLOT(setbackop(qreal)));
    connect(swidget, SIGNAL(ramOpChanged(qreal)), this, SLOT(setramop(qreal)));
    connect(swidget, SIGNAL(swapOpChanged(qreal)), this, SLOT(setswapop(qreal)));
    swidget->show();
}

void memmon::reloadcolors(){
    QColor backcolor = string_to_color(settings->value("backgroundcolor", "0,0,0").toString());
    QColor ramcolor = string_to_color(settings->value("RAMcolor", "255,0,0").toString());
    QColor swapcolor = string_to_color(settings->value("Swapcolor", "100,0,0").toString());
    QList<QColor> colorlist = {ramcolor, swapcolor};

    gwidget->backcolor = backcolor;
    gwidget->colors = colorlist;
    gwidget->update();
}

void memmon::setbackop(qreal opacity){
    gwidget->backopacity = opacity;
    gwidget->update();
}

void memmon::setramop(qreal opacity){
    gwidget->opacitys[0] = opacity;
    gwidget->update();
}

void memmon::setswapop(qreal opacity){
    gwidget->opacitys[1] = opacity;
    gwidget->update();
}

void memmon::reloadsettings(){
    loadsettings();
}

void memmon::updatemem(){
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    //get total memory
    QString memtotal = file.readLine();
    memtotal.remove("MemTotal:");
    memtotal.chop(3);
    memtotal.remove(" ");

    file.readLine();//skip line

    //get available memory
    QString memfree = file.readLine();
    memfree.remove("MemAvailable:");
    memfree.chop(3);
    memfree.remove(" ");

    //skip down to swap
    for (int c = 0; c < 11; c++)
        file.readLine();

    //get total swap
    QString swaptotal = file.readLine();
    swaptotal.remove("SwapTotal:");
    swaptotal.chop(3);
    swaptotal.remove(" ");

    //get free swap
    QString swapfree = file.readLine();
    swapfree.remove("SwapFree:");
    swapfree.chop(3);
    swapfree.remove(" ");

    QList<qreal> values;
    if (swapbehavior == 0){ // disabled
        double free = memfree.toDouble();
        double total = memtotal.toDouble();
        values << (total - free) / total;
    }
    else if (swapbehavior == 1){ // combine
        double free = memfree.toDouble() + swapfree.toDouble();
        double total = memtotal.toDouble() + swaptotal.toDouble();
        values << (total - free) / total;
    }
    else { // seperate
        double mfree = memfree.toDouble();
        double mtotal = memtotal.toDouble();
        values << (mtotal - mfree) / mtotal;

        double sfree = swapfree.toDouble();
        double stotal = swapfree.toDouble();
        values << (stotal - sfree) / stotal;
    }
    gwidget->updategraph(values);
}

void memmon::runcommand(){
    if (clickedcommand != "")
        QProcess::startDetached(clickedcommand);
}
