#include "settingswidget.h"
#include "ui_settingswidget.h"

#include "miscutills.h"

using namespace miscutills;

settingswidget::settingswidget(QWidget *parent) : QWidget(parent), ui(new Ui::settingswidget){
    ui->setupUi(this);
    settings = new QSettings("Forest", "CPU Monitor");
    loadsettings();
}

settingswidget::~settingswidget(){
    delete ui;
}

void settingswidget::loadsettings(){
    backcolor = string_to_color(settings->value("backgroundcolor", "0,0,0").toString());
    ui->backgroundcolorbt->setIcon(make_color_icon(backcolor));
    forecolor = string_to_color(settings->value("foregroundcolor", "255,255,255").toString());
    ui->forecolorbt->setIcon(make_color_icon(forecolor));
    ui->backgroundopslider->setValue(int(settings->value("backgroundopacity", 1).toDouble() * 100));
    ui->foreopslider->setValue(int(settings->value("foregroundopacity", 1).toDouble() * 100));
    ui->commandTbox->setText(settings->value("command").toString());
    ui->updateintervalSbox->setValue(settings->value("updateinterval", 1000).toInt());
    ui->widthSbox->setValue(settings->value("width", 40).toInt());
}

void settingswidget::on_backgroundcolorbt_clicked(){
    QColorDialog cdialog(this);
    cdialog.setCurrentColor(backcolor);
    cdialog.setWindowTitle("Background Color");
    if (cdialog.exec())
    {
        backcolor = cdialog.selectedColor();
        ui->backgroundcolorbt->setIcon(make_color_icon(backcolor));
        settings->setValue("backgroundcolor", color_to_string(backcolor));
        emit colorschanged();
    }
}

void settingswidget::on_forecolorbt_clicked(){
    QColorDialog cdialog(this);
    cdialog.setCurrentColor(forecolor);
    cdialog.setWindowTitle("Foreground Color");
    if (cdialog.exec()){
        forecolor = cdialog.selectedColor();
        ui->forecolorbt->setIcon(make_color_icon(forecolor));
        settings->setValue("foregroundcolor", color_to_string(forecolor));
        emit colorschanged();
    }
}

void settingswidget::on_backgroundopslider_valueChanged(int value){
    qreal opacity = qreal(value) / 100;
    settings->setValue("backgroundopacity", opacity);
    emit backOpChanged(opacity);
}

void settingswidget::on_foreopslider_valueChanged(int value){
    qreal opacity = qreal(value) / 100;
    settings->setValue("foregroundopacity", opacity);
    emit foreOpChanged(opacity);
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
    close();
}
