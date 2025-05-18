/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021-2024 Nicholas Yoder
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

#include <QFileSystemWatcher>

#include "fvolume.h"
#include "settingswidget.h"

fvolume::fvolume(){}

void fvolume::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist){
    layout->addWidget(this);
    setupIconButton("audio-volume-muted");

    save_runner = new RunOnce(1000);
    connect(save_runner, &RunOnce::activated, this, &fvolume::save_volumes);

    popup_layout = new QVBoxLayout;
    popupbox = new popup(popup_layout, this, CenteredOnWidget);

    pmenu = new popupmenu(this, CenteredOnWidget);
    foreach (pmenuitem *item, itemlist)
        pmenu->additem(item);

    pmenu->addseperator();
    pmenuitem *settings_item = new pmenuitem("Manage devices", QIcon::fromTheme("preferences-sound"));
    connect(settings_item, &pmenuitem::clicked, this, &fvolume::showsettings);
    pmenu->additem(settings_item);
    pmenuitem *refresh_item = new pmenuitem("Re-scan devices", QIcon::fromTheme("reload"));
    connect(refresh_item, &pmenuitem::clicked, this, &fvolume::loadsettings);
    pmenu->additem(refresh_item);
    pmenuitem *muted_item = new pmenuitem("Toggle Muted", QIcon::fromTheme("audio-volume-muted"));
    connect(muted_item, &pmenuitem::clicked, this, &fvolume::togglemuted);
    pmenu->additem(muted_item);

    connect(this, &fvolume::leftclicked, popupbox, &popup::showpopup);
    connect(this, &fvolume::rightclicked, pmenu, &popupmenu::show);
    connect(this, &fvolume::mouseReleased, this, &fvolume::handlemouseReleased);

    loadsettings();
    updateicon();

    QFileSystemWatcher *watcher = new QFileSystemWatcher(this);
    watcher->addPath("/dev/snd");
    RunOnce* runner = new RunOnce(1000);
    connect(watcher, &QFileSystemWatcher::directoryChanged, runner, &RunOnce::try_activate);
    connect(runner, &RunOnce::activated, this, &fvolume::loadsettings);

    QDBusConnection::sessionBus().registerObject("/org/forest/panel/volume", this, QDBusConnection::ExportScriptableSlots);
}

QHash<QString, QString> fvolume::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "Volume control";
    return info;
}

void fvolume::wheelEvent(QWheelEvent *event){
    if (event->angleDelta().y() > 1)
        master_device->setVolume(master_device->volume() + 2);
    else
        master_device->setVolume(master_device->volume() - 2);
}

void fvolume::loadsettings(){
    if(audioengine)
        delete audioengine;
    audioengine = new AlsaEngine(this);

    QSettings settings("Forest", "Volume Manager");
    settings.sync();
    autosave = settings.value("autosave", true).toBool();

    AudioDevice * first_dev = audioengine->sinks().first();
    QString master = settings.value("master", (first_dev) ? first_dev->description() : "").toString();

    // Clear the existing layout
    QLayoutItem *item;
    while ((item = popup_layout->takeAt(0)) != nullptr) {
        if (item->layout()) {
            QLayout *layout = item->layout();
            QLayoutItem *childItem;
            while ((childItem = layout->takeAt(0)) != nullptr) {
                if (childItem->widget()) {
                    delete childItem->widget();
                }
                delete childItem;
            }
        } else if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    // Load audio devices
    foreach(AudioDevice *dev, audioengine->sinks()){
        if(dev->description() == master) master_device = dev;

        if (settings.value(dev->description() + "/show", true).toBool() == true){
            QVBoxLayout *device_layout = new QVBoxLayout;
            QLabel* label = new QLabel(dev->name());
            device_layout->addWidget(label);
            QSlider *slider = new QSlider(Qt::Horizontal);
            device_layout->addWidget(slider);
            slider->setRange(0, audioengine->volumeMax(dev));
            if(autosave){
                int current_volume = dev->volume();
                int saved_volume = settings.value(dev->description() + "/volume", current_volume).toInt();
                slider->setValue(saved_volume);
                if(current_volume != saved_volume)
                    dev->setVolume(saved_volume);
                connect(dev, &AudioDevice::volumeChanged, save_runner, &RunOnce::try_activate);
            }
            else{
                slider->setValue(dev->volume());
            }
            connect(slider, &QSlider::valueChanged, dev, &AudioDevice::setVolume);
            connect(dev, &AudioDevice::volumeChanged, slider, &QSlider::setValue);
            popup_layout->addLayout(device_layout);
        }
    }

    if(master_device){
        connect(master_device, &AudioDevice::volumeChanged, this, &fvolume::volumechanged);
        connect(master_device, &AudioDevice::muteChanged, this, &fvolume::mutechanged);
        master_volume = master_device->volume();
        master_muted = master_device->mute();
    }
}

void fvolume::showsettings(){
    // rescan devices and reload
    loadsettings();

    QStringList sink_list;
    foreach(AudioDevice *dev, audioengine->sinks())
        sink_list.append(dev->description());

    SettingsWidget* s_widget = new SettingsWidget(sink_list);
    connect(s_widget, &SettingsWidget::settings_changed, this, &fvolume::loadsettings);
    s_widget->show();
}

void fvolume::save_volumes(){
    QSettings settings("Forest", "Volume Manager");
    foreach(AudioDevice *dev, audioengine->sinks()){
        if (settings.value(dev->description() + "/show", true).toBool() == true)
            settings.setValue(dev->description() + "/volume", dev->volume());
    }
}

void fvolume::setvolume(int value){
    master_device->setVolume(value);
}

void fvolume::togglemuted(){
    master_device->setMute(!master_muted);
}

void fvolume::volumechanged(int value){
    master_volume = value;
    updateicon();
}

void fvolume::mutechanged(bool state){
    master_muted = state;
    updateicon();
}

void fvolume::updateicon(){
    qreal max = audioengine->volumeMax(master_device);
    qreal lowmax = max * 0.33;
    qreal mediummax = max * 0.66;

    QString ico;
    if (master_muted == true){
        ico = "audio-volume-muted";
    }
    else{
        if (master_volume < 1)
            ico = "audio-volume-muted";
        else if (master_volume < lowmax)
            ico = "audio-volume-low";
        else if (master_volume < mediummax)
            ico = "audio-volume-medium";
        else
            ico = "audio-volume-high";
    }

    setIcon(geticon(ico));
}

void fvolume::handlemouseReleased(QMouseEvent *event){
    if (event->button() == Qt::MiddleButton)
        togglemuted();
}
