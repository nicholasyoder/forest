// SPDX-License-Identifier: LGPL-3.0-or-later

#include "volumesettings.h"

#include <QCheckBox>
#include <QComboBox>

#include "alsaengine.h"
#include "audiodevice.h"
#include "listwidget.h"
#include "miscutills.h"
#include "settingsbinder.h"
#include "volumeconfig.h"

QList<settings_page*> VolumeSettings::pages()
{
    settings_page *page = new settings_page(volumeconfig::path, "Volume Control", "preferences-sound");
    page->set_keywords({"sound", "audio", "alsa", "mixer", "master", "device", "speaker", "headphone"});

    SettingsBinder *binder = new SettingsBinder("Forest", "Volume Manager", QString(), this);
    binder->set_callback([]{ miscutills::call_dbus("forest/panel/reloadappletsettings", {QString(volumeconfig::path)}); });

    settings_widget_group *behavior = new settings_widget_group("Behavior");
    QComboBox *master_combo = new QComboBox;
    binder->bind_custom(master_combo, &QComboBox::currentIndexChanged,
        [master_combo](QSettings &settings){
            int index = master_combo->findText(settings.value(volumeconfig::master).toString());
            master_combo->setCurrentIndex(qMax(index, 0));
        },
        [master_combo](QSettings &settings){ settings.setValue(volumeconfig::master, master_combo->currentText()); });
    behavior->add_child(new settings_widget("Master device", "Controlled by the panel icon, scrolling and volume keys", master_combo));
    QCheckBox *autosave_check = new QCheckBox;
    binder->bind(autosave_check, volumeconfig::autosave, volumeconfig::autosave_default);
    behavior->add_child(new settings_widget("Remember volume", "Restore each device's level at login", autosave_check));
    page->add_child(behavior);

    settings_widget_group *devices = new settings_widget_group("Shown in popup");
    ListWidget *device_list = new ListWidget;
    binder->bind_custom(device_list, &QListWidget::itemChanged,
        [device_list](QSettings &settings){
            for (int i = 0; i < device_list->count(); i++) {
                QListWidgetItem *item = device_list->item(i);
                bool shown = settings.value(volumeconfig::show(item->text()), volumeconfig::show_default).toBool();
                item->setCheckState(shown ? Qt::Checked : Qt::Unchecked);
            }
        },
        [device_list](QSettings &settings){
            for (int i = 0; i < device_list->count(); i++) {
                QListWidgetItem *item = device_list->item(i);
                settings.setValue(volumeconfig::show(item->text()), item->checkState() == Qt::Checked);
            }
        });
    devices->add_child(new settings_widget("", "", device_list));
    page->add_child(devices);

    // Devices come and go, so list them on each open, before loading.
    connect(page, &settings_page::opened, this, [master_combo, device_list]{
        AlsaEngine engine;
        QSignalBlocker combo_blocker(master_combo);
        QSignalBlocker list_blocker(device_list);
        master_combo->clear();
        device_list->clear();
        foreach (AudioDevice *dev, engine.sinks()) {
            master_combo->addItem(dev->description());
            QListWidgetItem *item = new QListWidgetItem(dev->description());
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
            device_list->addItem(item);
        }
    });
    connect(page, &settings_page::opened, binder, &SettingsBinder::load);

    return {page};
}
