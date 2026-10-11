// SPDX-License-Identifier: LGPL-3.0-or-later

#include "batterymonitor.h"

enum class chargestate { Discharging, Charging, Charged };

// Most to least granular naming scheme; each name plain before -symbolic, since
// Qt doesn't recolour symbolic icons.
static QStringList iconnames(qreal fraction, chargestate state){
    const int level = qBound(0, qRound(fraction * 10), 10) * 10;
    const QString suffix = state == chargestate::Charging ? "-charging" : "";
    QStringList names;

    if (state == chargestate::Charged)
        names << QString("battery-%1-charged").arg(level, 3, 10, QChar('0'));
    names << QString("battery-%1%2").arg(level, 3, 10, QChar('0')).arg(suffix);

    if (state == chargestate::Charged)
        names << QString("battery-level-%1-charged").arg(level);
    names << QString("battery-level-%1%2").arg(level).arg(suffix);

    const QString word = fraction < 0.1 ? "caution" : fraction < 0.2 ? "low" : fraction < 0.8 ? "good" : "full";
    if (state == chargestate::Charged)
        names << "battery-full-charged";
    names << "battery-" + word + suffix;

    names << "battery";

    QStringList withsymbolic;
    for (const QString &name : names)
        withsymbolic << name << name + "-symbolic";
    return withsymbolic;
}

batterymonitor::batterymonitor() : panelbutton(Icon)
{

}

batterymonitor::~batterymonitor()
{
    qDeleteAll(batterylist);
}

void batterymonitor::setupPlug(QBoxLayout *layout, QList<QAction*> itemlist)
{
    layout->addWidget(this);

    QDir psdir(PATH_TO_PS_DIR);
    foreach (QString sdir, psdir.entryList())
    {
        if (sdir.startsWith("bat", Qt::CaseInsensitive))
        {
            battery *bat = new battery(PATH_TO_PS_DIR + sdir);
            bat->refresh();
            batterylist.append(bat);
        }
    }

    updateicon();
    if (!batterylist.isEmpty())
    {
        connect(updatetimer, &QTimer::timeout, this, &batterymonitor::updatedata);
        updatetimer->start(3000);
    }

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(popuplabel);
    pbox = new popup(vlayout, this, CenteredOnWidget);
    connect(this, &panelbutton::leftclicked, this, &batterymonitor::showpopup);
}

void batterymonitor::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ThemeChange)
    {
        iconcandidates.clear();
        updateicon();
    }
    panelbutton::changeEvent(event);
}

void batterymonitor::updatedata()
{
    foreach (battery *bat, batterylist)
    {
        bat->refresh();
    }
    updateicon();
}

void batterymonitor::updateicon()
{
    QStringList candidates;
    if (batterylist.isEmpty())
        candidates << "battery-missing" << "battery-missing-symbolic";
    else
    {
        // Combined like UPower's display device.
        qreal now = 0, full = 0;
        bool charging = false, allfull = true;
        foreach (battery *bat, batterylist)
        {
            now += bat->getnow();
            full += bat->getfull();
            charging |= bat->getstatus() == "Charging";
            allfull &= bat->getstatus() == "Full";
        }
        chargestate state = charging ? chargestate::Charging : allfull ? chargestate::Charged : chargestate::Discharging;
        candidates = iconnames(full > 0 ? now / full : 0, state);
    }

    if (candidates == iconcandidates)
        return;
    iconcandidates = candidates;

    for (const QString &name : candidates)
    {
        if (QIcon::hasThemeIcon(name))
        {
            setIcon(QIcon::fromTheme(name));
            return;
        }
    }
    setIcon(QIcon());
}

void batterymonitor::showpopup()
{
    QStringList entries;
    int c = 0;
    foreach (battery *bat, batterylist)
    {
        entries << "Battery " + QString::number(c) + "\n" + bat->getstatus() + "\n"
                   + QString::number(qRound(bat->getpercentfull()*100)) + "% full";
        c++;
    }
    popuplabel->setText(entries.join("\n\n"));
    pbox->showpopup();
}
