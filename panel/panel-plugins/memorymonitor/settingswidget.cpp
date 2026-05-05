// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingswidget.h"
#include "ui_settingswidget.h"

#include "miscutills.h"

using namespace miscutills;

settingswidget::settingswidget(QWidget *parent) : QWidget(parent), ui(new Ui::settingswidget){
    ui->setupUi(this);
    settings = new QSettings("Forest", "Memory Monitor");
    loadsettings();
}

settingswidget::~settingswidget(){
    delete ui;
}

void settingswidget::loadsettings(){
    backcolor = string_to_color(settings->value("backgroundcolor", "0,0,0").toString());
    ui->backgroundcolorbt->setIcon(make_color_icon(backcolor));
    RAMcolor = string_to_color(settings->value("RAMcolor", "255,0,0").toString());
    ui->ramcolorbt->setIcon(make_color_icon(RAMcolor));
    Swapcolor = string_to_color(settings->value("Swapcolor", "100,100,0").toString());
    ui->swapcolorbt->setIcon(make_color_icon(Swapcolor));

    if (settings->value("swapbehavior", "combine").toString() == "disabled")
        ui->swapdisabledRbt->setChecked(true);
    else if (settings->value("swapbehavior", "combine").toString() == "combine")
        ui->swapcombineRbt->setChecked(true);
    else if (settings->value("swapbehavior", "combine").toString() == "showseperate")
        ui->swapShowseperateRbt->setChecked(true);

    ui->backgroundopslider->setValue(int(settings->value("backgroundopacity", 1).toDouble() * 100));
    ui->ramopslider->setValue(int(settings->value("RAMopacity", 1).toDouble() * 100));
    ui->swapopslider->setValue(int(settings->value("Swapopacity", 1).toDouble() * 100));
    ui->commandTbox->setText(settings->value("command").toString());
    ui->updateintervalSbox->setValue(settings->value("updateinterval", 1000).toInt());
    ui->widthSbox->setValue(settings->value("width", 40).toInt());
}

void settingswidget::on_backgroundcolorbt_clicked(){
    QColorDialog cdialog(this);
    cdialog.setCurrentColor(backcolor);
    cdialog.setWindowTitle("Background Color");
    if (cdialog.exec()){
        backcolor = cdialog.selectedColor();
        ui->backgroundcolorbt->setIcon(make_color_icon(backcolor));
        settings->setValue("backgroundcolor", color_to_string(backcolor));
        emit colorschanged();
    }
}

void settingswidget::on_ramcolorbt_clicked(){
    QColorDialog cdialog(this);
    cdialog.setCurrentColor(RAMcolor);
    cdialog.setWindowTitle("RAM Color");
    if (cdialog.exec()){
        RAMcolor = cdialog.selectedColor();
        ui->ramcolorbt->setIcon(make_color_icon(RAMcolor));
        settings->setValue("RAMcolor", color_to_string(RAMcolor));
        emit colorschanged();
    }
}

void settingswidget::on_swapcolorbt_clicked(){
    QColorDialog cdialog(this);
    cdialog.setCurrentColor(Swapcolor);
    cdialog.setWindowTitle("Swap Color");
    if (cdialog.exec()){
        Swapcolor = cdialog.selectedColor();
        ui->swapcolorbt->setIcon(make_color_icon(Swapcolor));
        settings->setValue("Swapcolor", color_to_string(Swapcolor));
        emit colorschanged();
    }
}

void settingswidget::on_backgroundopslider_valueChanged(int value){
    qreal opacity = qreal(value) / 100;
    settings->setValue("backgroundopacity", opacity);
    emit backOpChanged(opacity);
}

void settingswidget::on_ramopslider_valueChanged(int value){
    qreal opacity = qreal(value) / 100;
    settings->setValue("RAMopacity", opacity);
    emit ramOpChanged(opacity);
}

void settingswidget::on_swapopslider_valueChanged(int value){
    qreal opacity = qreal(value) / 100;
    settings->setValue("Swapopacity", opacity);
    emit swapOpChanged(opacity);
}

void settingswidget::on_swapdisabledRbt_toggled(bool checked){
    if (checked){
        settings->setValue("swapbehavior", "disabled");
        emit settingschanged();
    }
}

void settingswidget::on_swapcombineRbt_toggled(bool checked){
    if (checked){
        settings->setValue("swapbehavior", "combine");
        emit settingschanged();
    }
}

void settingswidget::on_swapShowseperateRbt_toggled(bool checked){
    if (checked){
        settings->setValue("swapbehavior", "showseperate");
        emit settingschanged();
    }
}

void settingswidget::on_updateintervalSbox_editingFinished(){
    settings->setValue("updateinterval", ui->updateintervalSbox->value());
    emit settingschanged();
}

void settingswidget::on_commandTbox_editingFinished(){
    settings->setValue("command", ui->commandTbox->text());
    emit settingschanged();
}

void settingswidget::on_widthSbox_editingFinished(){
    settings->setValue("width", ui->widthSbox->value());
    emit settingschanged();
}

void settingswidget::on_okbt_clicked(){
    this->close();
}
