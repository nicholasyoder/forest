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

#include "fvolume.h"

fvolume::fvolume()
{

}

void fvolume::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist)
{
    layout->addWidget(this);

    audioengine = new AlsaEngine(this);
    audiodevice = audioengine->sinks().first();

    volume = audiodevice->volume();
    muted = audiodevice->mute();

    connect(audiodevice, SIGNAL(volumeChanged(int)), this, SLOT(volumechanged(int)));
    connect(audiodevice, SIGNAL(muteChanged(bool)), this, SLOT(mutechanged(bool)));

    setupIconButton("audio-volume-muted");

    QHBoxLayout *hlayout = new QHBoxLayout;
    slider = new QSlider(Qt::Vertical);
    hlayout->addStretch(5);
    hlayout->addWidget(slider);
    hlayout->addStretch(5);

    slider->setRange(0,audioengine->volumeMax(audiodevice));
    slider->setValue(volume);
    connect(slider, SIGNAL(valueChanged(int)), audiodevice, SLOT(setVolume(int)));
    popupbox = new popup(hlayout, this, CenteredOnWidget);
    popupbox->setFixedSize(50,150);

    pmenu = new popupmenu(this, CenteredOnWidget);
    foreach (pmenuitem *item, itemlist)
        pmenu->additem(item);

    pmenu->addseperator();
    pmenuitem *item = new pmenuitem("Toggle Muted", QIcon::fromTheme("audio-volume-muted"));
    connect(item, &pmenuitem::clicked, this, &fvolume::togglemuted);
    pmenu->additem(item);

    connect(this, &fvolume::leftclicked, popupbox, &popup::showpopup);
    connect(this, &fvolume::rightclicked, pmenu, &popupmenu::show);
    connect(this, &fvolume::mouseReleased, this, &fvolume::handlemouseReleased);

    updateicon();

    QDBusConnection::sessionBus().registerObject("/org/forest/panel/volume", this, QDBusConnection::ExportScriptableSlots);
}

QHash<QString, QString> fvolume::getpluginfo()
{
    QHash<QString, QString> info;
    info["name"] = "Volume control";
    return info;
}

void fvolume::wheelEvent(QWheelEvent *event){
    if (event->angleDelta().y() > 1)
        slider->setValue(slider->value() + 2);
    else
        slider->setValue(slider->value() - 2);
}

void fvolume::setvolume(int value)
{
    audiodevice->setVolume(value);
}

void fvolume::togglemuted()
{
    audiodevice->setMute(!muted);
}

void fvolume::volumechanged(int value)
{
    volume = value;
    updateicon();
}

void fvolume::mutechanged(bool state)
{
    muted = state;
    updateicon();
}

void fvolume::updateicon()
{
    qreal max = audioengine->volumeMax(audiodevice);
    qreal lowmax = max * 0.33;
    qreal mediummax = max * 0.66;

    QString ico;
    if (muted == true){
        ico = "audio-volume-muted";
    }
    else{
        if (volume < 1)
            ico = "audio-volume-muted";
        else if (volume < lowmax)
            ico = "audio-volume-low";
        else if (volume < mediummax)
            ico = "audio-volume-medium";
        else
            ico = "audio-volume-high";
    }

    setIcon(geticon(ico));
}

void fvolume::handlemouseReleased(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
        togglemuted();
}
