// SPDX-License-Identifier: LGPL-3.0-or-later

#include "memmon.h"
#include "memorymonitorconfig.h"
#include <QGenericPlugin>

using namespace miscutills;

memmon::memmon(){}
memmon::~memmon(){ delete pmenu; }

//beginning of plugin interface
void memmon::setupPlug(QBoxLayout *layout, QList<QAction*> itemlist){
    gwidget = new graphwidget;
    settings = new QSettings("Forest", "Memory Monitor");

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(QMargins(0,0,0,0));
    vlayout->addWidget(gwidget);
    setLayout(vlayout);
    layout->addWidget(this);

    pmenu = new QMenu;
    pmenu->addActions(itemlist);

    connect(this, &memmon::leftclicked, this, &memmon::runcommand);
    connect(this, &memmon::rightclicked, this, [this]{ popupMenuOnLauncher(pmenu, this, CenteredOnWidget); });

    loadsettings();
}

//end of plugin interface

void memmon::loadsettings(){
    namespace cfg = memorymonitorconfig;
    settings->sync();

    QColor backcolor = string_to_color(settings->value(cfg::backgroundcolor, color_to_string(cfg::backgroundcolor_default)).toString());
    QColor ramcolor = string_to_color(settings->value(cfg::ramcolor, color_to_string(cfg::ramcolor_default)).toString());
    QColor swapcolor = string_to_color(settings->value(cfg::swapcolor, color_to_string(cfg::swapcolor_default)).toString());
    QList<QColor> colorlist = {ramcolor, swapcolor};
    qreal backopacity = settings->value(cfg::backgroundopacity, 1).toDouble();
    qreal ramopacity = settings->value(cfg::ramopacity, 1).toDouble();
    qreal swapopacity = settings->value(cfg::swapopacity, 1).toDouble();
    QList<qreal> opacitylist = {ramopacity, swapopacity};

    QString sbehavior = settings->value(cfg::swapbehavior, cfg::swap_combine).toString();
    if (sbehavior == cfg::swap_disabled){
        swapbehavior = 0;
        gwidget->setupgraphs(1, colorlist, opacitylist, backcolor, backopacity);
    }
    else if (sbehavior == cfg::swap_separate){
        swapbehavior = 2;
        gwidget->setupgraphs(2, colorlist, opacitylist, backcolor, backopacity);
    }
    else{
        swapbehavior = 1;
        gwidget->setupgraphs(1, colorlist, opacitylist, backcolor, backopacity);
    }

    clickedcommand = settings->value(cfg::command, "").toString();

    delete refreshtimer;
    refreshtimer = new QTimer;
    connect(refreshtimer, &QTimer::timeout, this, &memmon::updatemem);
    refreshtimer->start(settings->value(cfg::updateinterval, cfg::updateinterval_default).toInt());

    gwidget->setFixedWidth(settings->value(cfg::width, cfg::width_default).toInt());
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
    if (clickedcommand != "") {
        QStringList args = QProcess::splitCommand(clickedcommand);
        if (!args.isEmpty()) {
            QString program = args.takeFirst();
            QProcess::startDetached(program, args);
        }
    }
}
