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

#include "widgetsensorconf.h"
#include "ui_widgetsensorconf.h"
#include <QSettings>

WidgetSensorConf::WidgetSensorConf(const QStringList &list,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetSensorConf)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose,true);

    sensorlist = list;
    ui->comboBox->addItems(list);

    loadsettings();
}

WidgetSensorConf::~WidgetSensorConf()
{
    delete ui;
}

void WidgetSensorConf::loadsettings()
{
    QSettings settings("Forest", "Temperature Monitor");
    settings.sync();
    ui->spinBox->setValue(settings.value("TimeUpdat",3).toInt());
    if (settings.value("Fahrenheit",false).toBool() == false)
    {
        ui->scaleCbox->setCurrentIndex(0);
    }
    else
    {
        ui->scaleCbox->setCurrentIndex(1);
    }
    ui->comboBox->setCurrentIndex(settings.value("ChipIndex",0).toInt());

    if (settings.value("display", "text").toString() == "text")
    {
        ui->textRbt->setChecked(true);
    }
    else
    {
        ui->barsRbt->setChecked(true);
    }

    ui->warningsbox->setValue(settings.value("warningtemp", 70).toInt());
    ui->criticalsbox->setValue(settings.value("criticaltemp", 90).toInt());

    foreach (QString sensor, sensorlist)
    {
        QStringList strings;
        strings << "" << sensor;
        QTreeWidgetItem *item = new QTreeWidgetItem(strings);
        if (settings.value(sensor + "Barenabled", true).toBool() == true)
        {
            item->setCheckState(0,Qt::Checked);
        }
        else
        {
            item->setCheckState(0,Qt::Unchecked);
        }
        ui->treeWidget->addTopLevelItem(item);
    }


    ui->heightsbox->setValue(settings.value("displayheight", 26).toInt());
    ui->barwidthsbox->setValue(settings.value("barwidth", 5).toInt());
    ui->barspacingsbox->setValue(settings.value("barspacing", 2).toInt());
    ui->marginsbox->setValue(settings.value("margin", 2).toInt());
    ui->maxtempsbox->setValue(settings.value("maxtemp", 110).toInt());

    backcolor = settings.value("backcolor").value<QColor>();
    QPixmap pix;
    QImage i(30,20, QImage::Format_ARGB32);
    i.fill(backcolor);
    pix = QPixmap::fromImage(i);
    ui->backcolorbt->setIcon(QIcon(pix));
}

void WidgetSensorConf::on_okbt_clicked()
{
    QSettings settings("Forest", "Temperature Monitor");
    settings.setValue("TimeUpdat",ui->spinBox->value());
    if (ui->scaleCbox->currentText() == "Celsius")
    {
        settings.setValue("Fahrenheit",false);
    }
    else
    {
        settings.setValue("Fahrenheit",true);
    }
    settings.setValue("ChipIndex",ui->comboBox->currentIndex());

    if (ui->textRbt->isChecked())
    {
        settings.setValue("display", "text");
    }
    else
    {
        settings.setValue("display", "bars");
    }

    settings.setValue("warningtemp", ui->warningsbox->value());
    settings.setValue("criticaltemp", ui->criticalsbox->value());

    int c = 0;
    while (c < ui->treeWidget->topLevelItemCount())
    {
        settings.setValue(ui->treeWidget->topLevelItem(c)->text(1) + "Barenabled", ui->treeWidget->topLevelItem(c)->checkState(0));
        c++;
    }

    settings.setValue("sensorlist", sensorlist);

    settings.setValue("displayheight", ui->heightsbox->value());
    settings.setValue("barwidth", ui->barwidthsbox->value());
    settings.setValue("barspacing", ui->barspacingsbox->value());
    settings.setValue("margin", ui->marginsbox->value());
    settings.setValue("maxtemp", ui->maxtempsbox->value());
    settings.setValue("backcolor", backcolor);

    emit settingSaved();
    //this->close();
}

void WidgetSensorConf::on_textRbt_toggled(bool checked)
{
    if (ui->textRbt->isChecked())
    {
        ui->stackedWidget->setCurrentIndex(0);
    }
    else
    {
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void WidgetSensorConf::on_backcolorbt_clicked()
{
    QColor backup = backcolor;
    backcolor = QColorDialog::getColor(backcolor, this, "Select Color", QColorDialog::ShowAlphaChannel);
    if (!backcolor.isValid())
    {
        backcolor = backup;
    }
    QPixmap pix;
    QImage i(30,20, QImage::Format_ARGB32);
    i.fill(backcolor);
    pix = QPixmap::fromImage(i);
    ui->backcolorbt->setIcon(QIcon(pix));
}
