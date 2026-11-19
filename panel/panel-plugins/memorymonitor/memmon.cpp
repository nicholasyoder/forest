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
    settings = new QSettings("Forest", "Memory Monitor", this);
    connect(refreshtimer, &QTimer::timeout, this, &memmon::updatemem);

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

    refreshtimer->start(qMax(50, settings->value(cfg::updateinterval, cfg::updateinterval_default).toInt()));

    gwidget->setFixedWidth(settings->value(cfg::width, cfg::width_default).toInt());
}


void memmon::updatemem(){
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    // Parsed by key, not line position; values are in kB.
    double memtotal = 0, memfree = 0, swaptotal = 0, swapfree = 0;
    // readAll(): atEnd() is true up front for /proc files (reported size 0).
    for (const QByteArray &line : file.readAll().split('\n')){
        const qsizetype colon = line.indexOf(':');
        if (colon < 0) continue;
        const QByteArrayView key = QByteArrayView(line).first(colon);
        const double value = line.mid(colon + 1).trimmed().split(' ').value(0).toDouble();
        if (key == "MemTotal") memtotal = value;
        else if (key == "MemAvailable") memfree = value;
        else if (key == "SwapTotal") swaptotal = value;
        else if (key == "SwapFree") swapfree = value;
    }

    auto fraction = [](double total, double free){ return total > 0 ? (total - free) / total : 0; };

    QList<qreal> values;
    if (swapbehavior == 0) // disabled
        values << fraction(memtotal, memfree);
    else if (swapbehavior == 1) // combine
        values << fraction(memtotal + swaptotal, memfree + swapfree);
    else // separate
        values << fraction(memtotal, memfree) << fraction(swaptotal, swapfree);
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
