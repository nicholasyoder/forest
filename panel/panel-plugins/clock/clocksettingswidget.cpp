#include "clocksettingswidget.h"
#include "ui_clocksettingswidget.h"

clocksettingswidget::clocksettingswidget(QWidget *parent) : QWidget(parent), ui(new Ui::clocksettingswidget){
    ui->setupUi(this);
    loadsettings();
}

clocksettingswidget::~clocksettingswidget(){
    delete ui;
}

void clocksettingswidget::loadsettings(){
    QSettings settings("Forest", "Clock");
    settings.sync();
    ui->twelvehourCHbox->setChecked(settings.value("12hour", true).toBool());
    ui->secondsCHbox->setChecked(settings.value("showseconds", false).toBool());
}

void clocksettingswidget::on_applybt_clicked(){
    QSettings settings("Forest", "Clock");
    settings.setValue("12hour", ui->twelvehourCHbox->isChecked());
    settings.setValue("showseconds", ui->secondsCHbox->isChecked());
    emit settingschanged();
}
