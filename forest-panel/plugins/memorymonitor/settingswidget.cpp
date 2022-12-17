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

#include "settingswidget.h"
#include "ui_settingswidget.h"

settingswidget::settingswidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::settingswidget)
{
    ui->setupUi(this);

    loadsettings();
}

settingswidget::~settingswidget()
{
    delete ui;
}

void settingswidget::loadsettings()
{
    QStringList sbackcolor = settings->value("backgroundcolor", "0,0,0").toString().split(",");
    backcolor.setRed(sbackcolor.at(0).toInt());
    backcolor.setGreen(sbackcolor.at(1).toInt());
    backcolor.setBlue(sbackcolor.at(2).toInt());
    QPixmap pix;
    QImage i(30,20, QImage::Format_ARGB32);
    i.fill(backcolor);
    pix = QPixmap::fromImage(i);
    ui->backgroundcolorbt->setIcon(QIcon(pix));


    QStringList sRAMcolor = settings->value("RAMcolor", "255,0,0").toString().split(",");
    RAMcolor.setRed(sRAMcolor.at(0).toInt());
    RAMcolor.setGreen(sRAMcolor.at(1).toInt());
    RAMcolor.setBlue(sRAMcolor.at(2).toInt());
    QPixmap pix2;
    QImage i2(30,20, QImage::Format_ARGB32);
    i2.fill(RAMcolor);
    pix2 = QPixmap::fromImage(i2);
    ui->ramcolorbt->setIcon(QIcon(pix2));

    QStringList sSwapcolor = settings->value("Swapcolor", "100,0,0").toString().split(",");
    Swapcolor.setRed(sSwapcolor.at(0).toInt());
    Swapcolor.setGreen(sSwapcolor.at(1).toInt());
    Swapcolor.setBlue(sSwapcolor.at(2).toInt());
    QPixmap pix3;
    QImage i3(30,20, QImage::Format_ARGB32);
    i3.fill(Swapcolor);
    pix3 = QPixmap::fromImage(i3);
    ui->swapcolorbt->setIcon(QIcon(pix3));

    if (settings->value("swapbehavior", "combine").toString() == "disabled")
        ui->swapdisabledRbt->setChecked(true);
    else if (settings->value("swapbehavior", "combine").toString() == "combine")
        ui->swapcombineRbt->setChecked(true);
    else if (settings->value("swapbehavior", "combine").toString() == "showseperate")
        ui->swapShowseperateRbt->setChecked(true);

    qreal backop = settings->value("backgroundopacity", 1).toDouble() * 100;
    ui->backgroundopslider->setValue(int(backop));

    qreal ramop = settings->value("RAMopacity", 1).toDouble() * 100;
    ui->ramopslider->setValue(int(ramop));

    qreal swapop = settings->value("Swapopacity", 1).toDouble() * 100;
    ui->swapopslider->setValue(int(swapop));

    ui->commandTbox->setText(settings->value("command").toString());
    ui->updateintervalSbox->setValue(settings->value("updateinterval", 1000).toInt());

    ui->widthSbox->setValue(settings->value("width", 40).toInt());
    ui->heightSbox->setValue(settings->value("height", 22).toInt());
    ui->marginSbox->setValue(settings->value("margin", 2).toInt());

    ui->resizetofitCbox->setChecked(settings->value("resizetofit", false).toBool());
}

void settingswidget::on_backgroundcolorbt_clicked()
{
    QColorDialog cdialog(this);
    cdialog.setCurrentColor(backcolor);
    cdialog.setWindowTitle("Background Color");
    if (cdialog.exec())
    {
        backcolor = cdialog.selectedColor();
        QPixmap pix;
        QImage i(30,20, QImage::Format_ARGB32);
        i.fill(backcolor);
        pix = QPixmap::fromImage(i);
        ui->backgroundcolorbt->setIcon(QIcon(pix));

        QString r;
        r.setNum(backcolor.red());
        QString g;
        g.setNum(backcolor.green());
        QString b;
        b.setNum(backcolor.blue());
        settings->setValue("backgroundcolor", r + "," + g + "," + b);

        emit colorschanged();
    }
}

void settingswidget::on_ramcolorbt_clicked()
{
    QColorDialog cdialog(this);
    cdialog.setCurrentColor(RAMcolor);
    cdialog.setWindowTitle("RAM Color");
    if (cdialog.exec())
    {
        RAMcolor = cdialog.selectedColor();
        QPixmap pix;
        QImage i(30,20, QImage::Format_ARGB32);
        i.fill(RAMcolor);
        pix = QPixmap::fromImage(i);
        ui->ramcolorbt->setIcon(QIcon(pix));

        QString r;
        r.setNum(RAMcolor.red());
        QString g;
        g.setNum(RAMcolor.green());
        QString b;
        b.setNum(RAMcolor.blue());
        settings->setValue("RAMcolor", r + "," + g + "," + b);

        emit colorschanged();
    }
}

void settingswidget::on_swapcolorbt_clicked()
{
    QColorDialog cdialog(this);
    cdialog.setCurrentColor(Swapcolor);
    cdialog.setWindowTitle("Swap Color");
    if (cdialog.exec())
    {
        Swapcolor = cdialog.selectedColor();
        QPixmap pix;
        QImage i(30,20, QImage::Format_ARGB32);
        i.fill(Swapcolor);
        pix = QPixmap::fromImage(i);
        ui->swapcolorbt->setIcon(QIcon(pix));

        QString r;
        r.setNum(Swapcolor.red());
        QString g;
        g.setNum(Swapcolor.green());
        QString b;
        b.setNum(Swapcolor.blue());
        settings->setValue("Swapcolor", r + "," + g + "," + b);

        emit colorschanged();
    }
}

void settingswidget::on_backgroundopslider_valueChanged(int value)
{
    qreal opacity = value;
    opacity = opacity / 100;
    settings->setValue("backgroundopacity", opacity);
    emit backOpChanged(opacity);
}

void settingswidget::on_ramopslider_valueChanged(int value)
{
    qreal opacity = value;
    opacity = opacity / 100;
    settings->setValue("RAMopacity", opacity);
    emit ramOpChanged(opacity);
}

void settingswidget::on_swapopslider_valueChanged(int value)
{
    qreal opacity = value;
    opacity = opacity / 100;
    settings->setValue("Swapopacity", opacity);
    emit swapOpChanged(opacity);
}

void settingswidget::on_swapdisabledRbt_toggled(bool checked)
{
    if (checked)
    {
        settings->setValue("swapbehavior", "disabled");
        emit settingschanged();
    }
}

void settingswidget::on_swapcombineRbt_toggled(bool checked)
{
    if (checked)
    {
        settings->setValue("swapbehavior", "combine");
        emit settingschanged();
    }
}

void settingswidget::on_swapShowseperateRbt_toggled(bool checked)
{
    if (checked)
    {
        settings->setValue("swapbehavior", "showseperate");
        emit settingschanged();
    }
}

void settingswidget::on_updateintervalSbox_editingFinished()
{
    settings->setValue("updateinterval", ui->updateintervalSbox->value());
    emit settingschanged();
}

void settingswidget::on_commandTbox_editingFinished()
{
    settings->setValue("command", ui->commandTbox->text());
    emit settingschanged();
}

void settingswidget::on_widthSbox_editingFinished()
{
    settings->setValue("width", ui->widthSbox->value());
    emit settingschanged();
}

void settingswidget::on_heightSbox_editingFinished()
{
    settings->setValue("height", ui->heightSbox->value());
    emit settingschanged();
}

void settingswidget::on_marginSbox_editingFinished()
{
    settings->setValue("margin", ui->marginSbox->value());
    emit settingschanged();
}

void settingswidget::on_okbt_clicked()
{
    this->close();
}

void settingswidget::on_resizetofitCbox_stateChanged(int arg1)
{
    settings->setValue("resizetofit", ui->resizetofitCbox->isChecked());
    emit settingschanged();
}
