// SPDX-License-Identifier: LGPL-3.0-or-later

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
    if (!master_device) return;

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
    AudioDevice *fallback_master = nullptr;
    foreach(AudioDevice *dev, audioengine->sinks()){
        qDebug() << "Detected audio device: " << dev->name() << dev->description();

        if(!fallback_master) fallback_master = dev; // set first device found to be the fallback master device
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

    if(!master_device) master_device = fallback_master; // set master to fallback if no device matched the master saved in the settings
    if(master_device){ // fallback could also be null if no audio devices were found
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
    if (master_device)
        master_device->setVolume(value);
}

void fvolume::togglemuted(){
    if (master_device)
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
