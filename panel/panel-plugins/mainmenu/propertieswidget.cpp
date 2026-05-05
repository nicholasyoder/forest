// SPDX-License-Identifier: LGPL-3.0-or-later

#include "propertieswidget.h"
#include "ui_propertieswidget.h"

propertieswidget::propertieswidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::propertieswidget)
{
    ui->setupUi(this);
}

propertieswidget::~propertieswidget()
{
    delete ui;
}

void propertieswidget::setdata(QString name, QString command, QString comment, QString icon, QString filepath)
{
    ui->nameTbox->setText(name);
    ui->commandTbox->setText(command);
    ui->commentTbox->setText(comment);
    ui->iconTbox->setText(icon);
    ui->filepathTbox->setText(filepath);

}
