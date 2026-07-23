// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cpumon.h"
#include <QGenericPlugin>

using namespace miscutills;

cpumon::cpumon(){}
cpumon::~cpumon(){}

//beginning of plugin interface
void cpumon::setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist){
    gwidget = new graphwidget;
    settings = new QSettings("Forest", "CPU Monitor");

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(QMargins(0,0,0,0));
    vlayout->addWidget(gwidget);
    setLayout(vlayout);
    layout->addWidget(this);

    pmenu = new popupmenu(this, CenteredOnWidget);
    foreach (pmenuitem *item, itemlist){
        pmenu->additem(item);
    }
    pmenu->addseperator();
    pmenuitem *item = new pmenuitem("CPU Monitor Settings", QIcon::fromTheme("configure"));
    connect(item, &pmenuitem::clicked, this, &cpumon::showsettingswidget);
    pmenu->additem(item);

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

    QColor backcolor = string_to_color(settings->value("backgroundcolor", "0,0,0").toString());
    QColor forecolor = string_to_color(settings->value("foregroundcolor", "255,255,255").toString());
    qreal backopacity = settings->value("backgroundopacity", 1).toDouble();
    qreal foreopacity = settings->value("foregroundopacity", 1).toDouble();
    gwidget->setupgraphs(1, {forecolor}, {foreopacity}, backcolor, backopacity);
    gwidget->setFixedWidth(settings->value("width", 40).toInt());
    clickedcommand = settings->value("command", "").toString();

    delete refreshtimer;
    refreshtimer = new QTimer;
    connect(refreshtimer, &QTimer::timeout, this, &cpumon::updatecpu);
    refreshtimer->start(settings->value("refreshrate", 500).toInt());
}

void cpumon::showsettingswidget(){
    settingswidget *swidget = new settingswidget;
    connect(swidget, &settingswidget::colorschanged, this, &cpumon::reloadcolors);
    connect(swidget, &settingswidget::settingschanged, this, &cpumon::reloadsettings);
    connect(swidget, &settingswidget::backOpChanged, this, &cpumon::setbackop);
    connect(swidget, &settingswidget::foreOpChanged, this, &cpumon::setforeop);
    swidget->setAttribute(Qt::WA_DeleteOnClose);
    swidget->show();
}

void cpumon::reloadcolors(){
    gwidget->backcolor = string_to_color(settings->value("backgroundcolor", "0,0,0").toString());
    gwidget->colors = {string_to_color(settings->value("foregroundcolor", "255,255,255").toString())};
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
