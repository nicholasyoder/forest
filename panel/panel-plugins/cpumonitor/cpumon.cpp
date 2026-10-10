// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cpumon.h"
#include "cpumonitorconfig.h"
#include <QGenericPlugin>

using namespace miscutills;

cpumon::cpumon(){}
cpumon::~cpumon(){ delete pmenu; }

//beginning of plugin interface
void cpumon::setupPlug(QBoxLayout *layout, QList<QAction*> itemlist){
    gwidget = new graphwidget;
    settings = new QSettings("Forest", "CPU Monitor");

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(QMargins(0,0,0,0));
    vlayout->addWidget(gwidget);
    setLayout(vlayout);
    layout->addWidget(this);

    pmenu = new QMenu;
    pmenu->addActions(itemlist);

    connect(this, &cpumon::leftclicked, this, &cpumon::runcommand);
    connect(this, &cpumon::rightclicked, this, [this]{ popupMenuOnLauncher(pmenu, this, CenteredOnWidget); });

    loadsettings();
}

//end of plugin interface

void cpumon::loadsettings(){
    namespace cfg = cpumonitorconfig;
    settings->sync();

    QColor backcolor = string_to_color(settings->value(cfg::backgroundcolor, color_to_string(cfg::backgroundcolor_default)).toString());
    QColor forecolor = string_to_color(settings->value(cfg::foregroundcolor, color_to_string(cfg::foregroundcolor_default)).toString());
    qreal backopacity = settings->value(cfg::backgroundopacity, 1).toDouble();
    qreal foreopacity = settings->value(cfg::foregroundopacity, 1).toDouble();
    gwidget->setupgraphs(1, {forecolor}, {foreopacity}, backcolor, backopacity);
    gwidget->setFixedWidth(settings->value(cfg::width, cfg::width_default).toInt());
    clickedcommand = settings->value(cfg::command, "").toString();

    delete refreshtimer;
    refreshtimer = new QTimer;
    connect(refreshtimer, &QTimer::timeout, this, &cpumon::updatecpu);
    refreshtimer->start(settings->value(cfg::updateinterval, cfg::updateinterval_default).toInt());
}

void cpumon::runcommand(){
    if (clickedcommand != "") {
        QStringList args = QProcess::splitCommand(clickedcommand);
        if (!args.isEmpty()) {
            QString program = args.takeFirst();
            QProcess::startDetached(program, args);
        }
    }
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
