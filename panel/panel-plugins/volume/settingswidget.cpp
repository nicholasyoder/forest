// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingswidget.h"
#include "ui_settingswidget.h"

#include <QSettings>

SettingsWidget::SettingsWidget(QStringList devices) : ui(new Ui::SettingsWidget){
    ui->setupUi(this);
    setWindowTitle("Volume Settings");
    setWindowIcon(QIcon::fromTheme("preferences-sound"));

    QSettings settings("Forest", "Volume Manager");
    settings.sync();

    ui->master_select->addItems(devices);
    ui->master_select->setCurrentText(settings.value("master", devices.first()).toString());
    ui->autosave_checkBox->setChecked(settings.value("autosave", true).toBool());

    foreach(QString device, devices){
        QStringList strings;
        strings << "" << device;
        QTreeWidgetItem *item = new QTreeWidgetItem(strings);
        if (settings.value(device + "/show", true).toBool() == true)
            item->setCheckState(0,Qt::Checked);
        else
            item->setCheckState(0,Qt::Unchecked);
        ui->devices_widget->addTopLevelItem(item);
    }
}

SettingsWidget::~SettingsWidget(){
    delete ui;
}

void SettingsWidget::save_settings(){
    QSettings settings("Forest", "Volume Manager");
    settings.setValue("master", ui->master_select->currentText());
    settings.setValue("autosave", ui->autosave_checkBox->isChecked());

    int c = 0;
    while (c < ui->devices_widget->topLevelItemCount()){
        settings.setValue(ui->devices_widget->topLevelItem(c)->text(1) + "/show", ui->devices_widget->topLevelItem(c)->checkState(0));
        c++;
    }
    emit settings_changed();
}

void SettingsWidget::on_applybt_clicked(){
    save_settings();
}

