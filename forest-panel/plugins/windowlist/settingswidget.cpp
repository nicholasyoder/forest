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
    QSettings settings("Forest", "Window List");
    settings.sync();

    QString bttype = settings.value("buttontype", "twopart").toString();
    if (bttype == "tab")
        ui->tabRbt->setChecked(true);
    else if (bttype == "icononly")
        ui->icononlyRbt->setChecked(true);
    else
        ui->textandiconRbt->setChecked(true);

    ui->maxsizesbox->setValue(settings.value("maxbuttonsize", 150).toInt());
    QString seps = settings.value("seperators", "None").toString();
}

void settingswidget::on_applybt_clicked()
{
    QSettings settings("Forest", "Window List");
    settings.setValue("maxbuttonsize",ui->maxsizesbox->value());

    if (ui->tabRbt->isChecked())
        settings.setValue("buttontype", "tab");
    else if (ui->icononlyRbt->isChecked())
        settings.setValue("buttontype", "icononly");
    else
        settings.setValue("buttontype", "twopart");

    emit settingschanged();
}
