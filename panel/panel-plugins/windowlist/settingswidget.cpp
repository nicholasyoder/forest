// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingswidget.h"
#include "ui_settingswidget.h"

settingswidget::settingswidget(QWidget *parent) : QWidget(parent), ui(new Ui::settingswidget){
    ui->setupUi(this);
    loadsettings();
}

settingswidget::~settingswidget(){
    delete ui;
}

void settingswidget::loadsettings(){
    QSettings settings("Forest", "Window List");
    settings.sync();

    ui->preview_cbox->setChecked(settings.value("showthumbnails", true).toBool());
    ui->maxsizesbox->setValue(settings.value("maxbuttonsize", 170).toInt());
    QString seps = settings.value("seperators", "None").toString();
}

void settingswidget::on_applybt_clicked(){
    QSettings settings("Forest", "Window List");
    settings.setValue("showthumbnails", ui->preview_cbox->isChecked());
    settings.setValue("maxbuttonsize",ui->maxsizesbox->value());
    settings.sync();
    emit settingschanged();
}
