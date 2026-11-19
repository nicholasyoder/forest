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
    settings = new QSettings("Forest", "CPU Monitor", this);
    connect(refreshtimer, &QTimer::timeout, this, &cpumon::updatecpu);

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

    refreshtimer->start(qMax(50, settings->value(cfg::updateinterval, cfg::updateinterval_default).toInt()));
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

    // "cpu user nice system idle iowait irq softirq steal guest guest_nice".
    // guest/guest_nice are already inside user/nice, so they're left out.
    // iowait counts as busy on purpose: it shows disk flushes (e.g. before unmounting a drive).
    const QStringList sections = QString(file.readLine()).split(QChar(' '), Qt::SkipEmptyParts);
    if (sections.size() < 5) return;

    unsigned long long total = 0, idle = 0;
    for (int c = 1; c < qMin(int(sections.size()), 9); c++){
        const unsigned long long v = sections[c].toULongLong();
        total += v;
        if (c == 4) idle = v; // idle only; iowait (c == 5) stays busy
    }
    const unsigned long long usage = total - idle;

    if (total > oldtotal){
        gwidget->updategraph({double(usage - oldvalue) / double(total - oldtotal)});
    }

    oldvalue = usage;
    oldtotal = total;
}
